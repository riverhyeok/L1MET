#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cassert>
#include <algorithm>
#include <hls_stream.h>
#include "firmware/puppimet.h"

// RUFL IO는 사용하지 않고 직접 파싱합니다.
// #include "utils/rufl_io.h" 

#ifndef NEVENTS
#define NEVENTS 5
#endif

// 디버그 출력 제어 (1: 켜기, 0: 끄기)
#define DEBUG_INPUT_LOG 1

// Phi Scale Factor (HLS와 동일)
const double MET_PHI_SCALE = 229.29936;

// ---------------------------------------------------------
// [Helper] Hex String -> ap_uint<64>
// ---------------------------------------------------------
ap_uint<64> hex_to_uint64(std::string hex_str) {
    if (hex_str.find("0x") == 0 || hex_str.find("0X") == 0) hex_str = hex_str.substr(2);
    if (hex_str.empty()) return 0;
    
    ap_uint<64> val = 0;
    std::stringstream ss;
    ss << std::hex << hex_str;
    uint64_t temp = 0;
    ss >> temp;
    val = temp;
    return val;
}

// ---------------------------------------------------------
// [Helper] Unpack Input Particle (64bit -> Particle_T)
// ---------------------------------------------------------
void unpack_input_particle(ap_uint<64> packed, Particle_T& p) {
    if (packed == 0) {
        p.hwPt = 0; p.hwEta = 0; p.hwPhi = 0;
        return;
    }
    // L1T Standard Format Assumption:
    // Pt [0:13] (14bits)
    // Eta [14:25] (12bits) 
    // Phi [26:36] (11bits) 
    p.hwPt  = packed.range(13, 0);
    p.hwEta = packed.range(25, 14);
    p.hwPhi = packed.range(36, 26);
}

// ---------------------------------------------------------
// [Helper] Unpack Reference Sum (64bit -> Sum Struct)
// ---------------------------------------------------------
struct RefSum {
    ap_uint<14> hwPt;    
    ap_uint<11> hwPhi;   
    ap_uint<14> hwSumPt; 
    
    double getFloatPt() { return hwPt.to_double(); }
    double getFloatPhi() { return hwPhi.to_double() / MET_PHI_SCALE; }
};

void unpack_reference_sum(ap_uint<64> packed, RefSum& s) {
    // Sum struct packing order (LSB First): hwPt -> hwPhi -> hwSumPt
    s.hwPt    = packed.range(13, 0);
    s.hwPhi   = packed.range(24, 14); 
    s.hwSumPt = packed.range(38, 25);
}

// ---------------------------------------------------------
// [Helper] Hardware Phi -> Float Radian
// ---------------------------------------------------------
double getFloatPhi(phi_t hwPhi) {
    return hwPhi.to_double() / MET_PHI_SCALE;
}

// ---------------------------------------------------------
// [Helper] File Parser
// ---------------------------------------------------------
std::vector<ap_uint<64>> parse_file(const char* filename, bool is_reference = false) {
    std::vector<ap_uint<64>> data;
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open " << filename << std::endl;
        return data;
    }

    std::string line;
    while(std::getline(inFile, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string s_clk, s_idx, s_data;
        
        ss >> s_clk;
        // 헤더(문자열) 스킵
        if (!isdigit(s_clk[0])) continue;
        
        ss >> s_idx >> s_data;
        if (s_data.empty()) continue;
        
        data.push_back(hex_to_uint64(s_data));
    }
    inFile.close();
    return data;
}

int main() {
    std::cout << "\n===================================================================" << std::endl;
    std::cout << "   L1 MET Ultimate Testbench: SW vs HLS vs Reference" << std::endl;
    std::cout << "===================================================================\n" << std::endl;

    // 1. Load Data
    std::cout << ">> Loading Inputs..." << std::endl;
    std::vector<ap_uint<64>> input_data_packed = parse_file("DeregionizerIn.txt");
    
    std::cout << ">> Loading Reference..." << std::endl;
    std::vector<ap_uint<64>> ref_data_packed = parse_file("METsOut.txt", true);

    std::cout << "   - Input Lines: " << input_data_packed.size() << std::endl;
    std::cout << "   - Ref Lines  : " << ref_data_packed.size() << " (Expected events)" << std::endl;

    // 2. Simulation Setup
    int total_frames = N_FRAMES; // 54
    int links = N_INPUT_LINKS;   // 36
    int entries_per_event = total_frames * links; // 1944

    // 이벤트 수 결정
    int events_to_run = std::min((int)ref_data_packed.size(), NEVENTS);
    
    if ((long long)events_to_run * entries_per_event > (long long)input_data_packed.size()) {
        std::cout << "!! WARNING: Not enough input data. Adjusting event count." << std::endl;
        events_to_run = input_data_packed.size() / entries_per_event;
    }

    // 3. Main Event Loop
    for (int iEvent = 0; iEvent < events_to_run; ++iEvent) {
        std::cout << "\n--- Event " << iEvent << " ---" << std::endl;

        // Variables for Comparison
        double sw_px = 0.0, sw_py = 0.0;
        int sw_valid_cnt = 0;

        hls::stream<Particle_T> in_streams[N_INPUT_LINKS];
        Particle_xy hls_xy_res;
        METCtrlToken token_d = {0,0,0}, token_q;
        Sum hw_met; // HLS Final Result

        // ----------------------------------------------------
        // Frame Loop (54 Clocks)
        // ----------------------------------------------------
        for (int frame = 0; frame < N_FRAMES; ++frame) {
            
            #if DEBUG_INPUT_LOG
                std::cout << " [Clk " << std::setw(2) << frame << "] In: ";
                bool has_data = false;
            #endif

            for (int link = 0; link < N_INPUT_LINKS; ++link) {
                long long idx = ((long long)iEvent * entries_per_event) + (frame * N_INPUT_LINKS) + link;
                
                ap_uint<64> packed_val = 0;
                if (idx < input_data_packed.size()) packed_val = input_data_packed[idx];
                
                Particle_T p;
                unpack_input_particle(packed_val, p);

                // [Debug Log] (Link:Hex)
                #if DEBUG_INPUT_LOG
                    if (packed_val != 0) {
                        std::cout << "(" << link << ":0x" 
                                  << std::uppercase << std::hex << packed_val << std::dec 
                                  << ") ";
                        has_data = true;
                    }
                #endif

                // [SW Calc] Accumulate valid particles
                if (p.hwPt > 0) {
                    double pt  = p.hwPt.to_double();
                    double phi = getFloatPhi(p.hwPhi); // HW Unit -> Radian
                    sw_px -= pt * std::cos(phi);
                    sw_py -= pt * std::sin(phi);
                    sw_valid_cnt++;
                }

                // [HLS Input]
                in_streams[link].write(p);
            }
            
            #if DEBUG_INPUT_LOG
                if(!has_data) std::cout << "(All 0)";
                std::cout << std::endl;
            #endif
            
            // Execute DUT
            puppimet_xy(in_streams, hls_xy_res, token_d, token_q);
        }

        // ----------------------------------------------------
        // Post-Processing
        // ----------------------------------------------------
        
        // 1. SW Finalize
        double sw_pt  = std::hypot(sw_px, sw_py);
        double sw_phi = std::atan2(sw_py, sw_px);

        // 2. HLS Output Conversion (XY -> PtPhi)
        double hls_px = hls_xy_res.hwPx.to_double();
        double hls_py = hls_xy_res.hwPy.to_double();
        double hls_pt_raw = std::hypot(hls_px, hls_py);
        double hls_phi_rad = std::atan2(hls_py, hls_px);
        
        hw_met.hwPt  = pt_t(hls_pt_raw);
        hw_met.hwPhi = phi_t(hls_phi_rad * MET_PHI_SCALE);

        // 3. Reference Unpack
        RefSum ref;
        unpack_reference_sum(ref_data_packed[iEvent], ref);

        // ----------------------------------------------------
        // Reporting
        // ----------------------------------------------------
        std::cout << std::fixed << std::setprecision(4);
        std::cout << "          |      Pt     |  Phi (Rad)  |  Phi (Raw)" << std::endl;
        std::cout << "----------+-------------+-------------+------------" << std::endl;
        
        // SW
        std::cout << "SW Calc   | " << std::setw(11) << sw_pt 
                  << " | " << std::setw(11) << sw_phi 
                  << " | " << std::setw(10) << (int)(sw_phi * MET_PHI_SCALE) 
                  << " (Valid Parts: " << sw_valid_cnt << ")" << std::endl;

        // HLS
        std::cout << "HLS Output| " << std::setw(11) << hw_met.hwPt.to_double() 
                  << " | " << std::setw(11) << getFloatPhi(hw_met.hwPhi)
                  << " | " << std::setw(10) << hw_met.hwPhi.to_int() << std::endl;

        // Reference
        std::cout << "Reference | " << std::setw(11) << ref.getFloatPt()
                  << " | " << std::setw(11) << ref.getFloatPhi()
                  << " | " << std::setw(10) << ref.hwPhi.to_int() << std::endl;

        // Comparison
        bool pt_match = std::abs(ref.getFloatPt() - hw_met.hwPt.to_double()) < 1.0;
        
        std::cout << "\n>> Result: " << (pt_match ? "MATCH" : "MISMATCH") << std::endl;
        if (!pt_match) {
            std::cout << "   [Analysis] Pt differs by " << std::abs(ref.getFloatPt() - hw_met.hwPt.to_double()) << std::endl;
            if (std::abs(sw_pt - hw_met.hwPt.to_double()) > 1.0) {
                std::cout << "   [Warning] SW also differs from HLS. Check HLS logic or Coefficients." << std::endl;
            } else if (std::abs(sw_pt - ref.getFloatPt()) > 1.0) {
                std::cout << "   [Warning] SW matches HLS, but both differ from Reference." << std::endl;
                std::cout << "             -> Likely Input Bit-Mapping issue or Reference Interpretation issue." << std::endl;
            }
        }
        std::cout << std::endl;
    }

    return 0;
}