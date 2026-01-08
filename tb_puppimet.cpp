#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <vector>
#include <numeric>
#include "utils/rufl_io.h"
#include "firmware/puppimet.h"


#ifndef N_INPUT_LINKS
#define N_INPUT_LINKS 36
#endif

#ifndef N_FRAMES
#define N_FRAMES 54
#endif

#ifndef NEVENTS
#define NEVENTS 5
#endif


double mean(const std::vector<double>& v) {
    if (v.empty()) return 0;
    double s = std::accumulate(v.begin(), v.end(), 0.0);
    return s / v.size();
}

double stddev(const std::vector<double>& v) {
    if (v.size() < 2) return 0;
    double m = mean(v);
    double acc = 0;
    for (double x : v) acc += (x - m)*(x - m);
    return std::sqrt(acc / (v.size() - 1));
}



int main() {
    std::cout << "\nStart L1 MET Testbench (Deregionizer Input Mode)\n" << std::endl;

    std::vector<std::vector<PuppiObj>> input_clocks;
    FILE* deregioFile = fopen("DeregionizerIn.txt", "r");
    if (deregioFile != NULL) {
        read_rufl_file<PuppiObj>(deregioFile, input_clocks, false);
        fclose(deregioFile);
    } else {
        std::cerr << "Error: Could not open DeregionizerIn.txt\n";
        return -1;
    }

    std::vector<std::vector<Sum>> met_ref;
    FILE* refFile = fopen("METsOut.txt", "r");
    if (refFile != NULL) {
        read_rufl_file<Sum>(refFile, met_ref, false);
        fclose(refFile);
    } else {
        std::cerr << "Error: Could not open METsOut.txt\n";
        return -1;
    }

    int n_Matched = 0;
    int n_Unmatched = 0;

    // --------------------------------------------------------
    // Statistics accumulation
    // --------------------------------------------------------
    std::vector<double> diff_pt_list;
    std::vector<double> diff_phi_list;
    std::vector<double> hls_pt_list;
    std::vector<double> hls_phi_list;


#ifdef MET_WRITE_TB_FILE
    FILE* HLS_MET_File = fopen("HLS_MET.txt", "w");
#endif

    // =========================================================
    // Event Loop
    // =========================================================
    for (int iEvent = 0; iEvent < NEVENTS; iEvent++) {
        std::cout << "Event " << iEvent << " Processing..." << std::endl;

        double sw_px = 0;
        double sw_py = 0;

        Particle_xy met_xy_out;
        Sum hw_met_final;
        METCtrlToken token_d, token_q, token_i;;

        // =====================================================
        // Frame Loop (54 clocks)
        // =====================================================
        for (int iFrame = 0; iFrame < N_FRAMES; iFrame++) {
            int gclk = iEvent * N_FRAMES + iFrame;
            if (gclk >= input_clocks.size()) break;

            Particle_T arr[N_INPUT_LINKS];
            std::vector<PuppiObj>& slice = input_clocks[gclk];

            for (int k = 0; k < N_INPUT_LINKS; k++) {
                if (k < slice.size()) {
                    arr[k].hwPt  = slice[k].hwPt;
                    arr[k].hwEta = slice[k].hwEta;
                    arr[k].hwPhi = slice[k].hwPhi;
                } else {
                    arr[k].hwPt  = 0;
                    arr[k].hwEta = 0;
                    arr[k].hwPhi = 0;
                }

                if (arr[k].hwPt > 0) {
                    double pt = arr[k].hwPt.to_float();
                    double phi = floatPhi(arr[k].hwPhi);
                    sw_px -= pt * cos(phi);
                    sw_py -= pt * sin(phi);
                }
            }

            puppimet_xy(arr, met_xy_out, token_d, token_q, token_i);
        }

        // =====================================================
        // Convert HLS XY → PT/PHI
        // =====================================================
        pxpy_to_ptphi(met_xy_out, hw_met_final, token_d, token_q);

        double hls_pt = hw_met_final.hwPt.to_float();
        double hls_phi = floatPhi(hw_met_final.hwPhi);

        double sw_pt = hypot(sw_px, sw_py);
        double sw_phi = atan2(sw_py, sw_px);

        // --------------------
        // Add statistics
        // --------------------
        hls_pt_list.push_back(hls_pt);
        hls_phi_list.push_back(hls_phi);

        diff_pt_list.push_back(hls_pt - sw_pt);
        diff_phi_list.push_back(hls_phi - sw_phi);

        bool isSame = false;
        if (iEvent < met_ref.size() && !met_ref[iEvent].empty()) {
            isSame = (met_ref[iEvent][0] == hw_met_final);
        }

        if (isSame) n_Matched++;
        else n_Unmatched++;

        // =====================================================
        // Preserve original per-event output
        // =====================================================
        std::cout << "Event " << iEvent << " Result:" << std::endl;

        std::cout << "  [HLS] Pt: " << hls_pt
                  << " Phi: " << hls_phi << std::endl;

        std::cout << "  [REF] Pt: " 
                  << met_ref[iEvent][0].hwPt.to_float()
                  << " Phi: "
                  << floatPhi(met_ref[iEvent][0].hwPhi)
                  << std::endl;

        std::cout << "  [SW ] Pt: " << sw_pt
                  << " Phi: " << sw_phi << std::endl;

        std::cout << "  Match: " << (isSame ? "TRUE" : "FALSE")
                  << "\n" << std::endl;


#ifdef MET_WRITE_TB_FILE
        std::vector<Sum> evout = {hw_met_final};
        write_rufl_event(HLS_MET_File, evout, iEvent, "HLS MET Out");
#endif
    }

#ifdef MET_WRITE_TB_FILE
    fclose(HLS_MET_File);
#endif

    // =========================================================
    // Final summary
    // =========================================================
    std::cout << "\nTotal Results:\n";
    std::cout << "Matched: " << n_Matched << std::endl;
    std::cout << "Unmatched: " << n_Unmatched << std::endl;

    // =========================================================
    // Final Statistics Output
    // =========================================================
    std::cout << "\n===== MET Statistics =====\n";

    std::cout << "Mean(HLS_pt - SW_pt): " << mean(diff_pt_list) << std::endl;
    std::cout << "StdDev(HLS_pt - SW_pt): " << stddev(diff_pt_list) << std::endl;

    std::cout << "Mean(HLS_phi - SW_phi): " << mean(diff_phi_list) << std::endl;
    std::cout << "StdDev(HLS_phi - SW_phi): " << stddev(diff_phi_list) << std::endl;

    std::cout << "StdDev(HLS_pt): " << stddev(hls_pt_list) << std::endl;
    std::cout << "StdDev(HLS_phi): " << stddev(hls_phi_list) << std::endl;

    return 0;
}
