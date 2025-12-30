#ifndef RUFL_IO_H__
#define RUFL_IO_H__

#include <vector>
#include <algorithm>
#include <numeric>
#include <string>

void write_rufl_header(FILE* file, const char* title){
    fprintf(file, "%s\n", title);
    fprintf(file, "%15s%15s%17s%12s%12s\n", "Clock", "Index", "Data", "FrameValid", "DataValid");
}

template<class T>
void write_rufl_line(FILE* file, T obj, int clock, int index, bool dataValid, bool frameValid){
    std::string dv = dataValid ? "TRUE" : "FALSE";
    std::string fv = frameValid ? "TRUE" : "FALSE";
    long long unsigned bits = obj.pack();
    fprintf(file, "%15d%15d %016llX%12s%12s\n", clock, index, bits, fv.c_str(), dv.c_str());
}

template<class T>
void write_rufl_event(FILE* file, std::vector<T> flat, int nEvent, const char* title){
    if(nEvent == 0){
        write_rufl_header(file, title);
    }
    int clock = 81 + 54 * nEvent;
    for(int i = 0; i < flat.size(); i++){
        write_rufl_line(file, flat.at(i), clock, i, true, true);
    }
}

template<class T>
void read_rufl_file(FILE* file, std::vector<std::vector<T>> &out, bool reshaped){
    int clock = -1;
    int lastclock = -1;
    int index = 0;
    long long unsigned data = 0;
    char fv[5]; // frame valid
    char dv[5]; // data valid
    std::vector<T> event;
    // Trim the two header lines
    fscanf(file, "%*[^\n]\n");
    fscanf(file, "%*[^\n]\n");
    while (!feof(file)){
        if (fscanf(file, " %d %d %llX %s %s\n", &clock, &index, &data, &fv, &dv) != 5) return;
        T part = T::unpack(data);
        bool same_event = reshaped
                            ? (clock - lastclock) <= 1 or lastclock == -1
                            : clock == lastclock or lastclock == -1;
        if(same_event){ // part belongs to same event, add to collection
            event.push_back(part);
        }else if(lastclock != -1){ // part belongs to new event, and not first read
            out.push_back(event);
            event.clear();
            event.push_back(part);
        }
        lastclock = clock;
    }
    if(event.size() != 0){
        out.push_back(event); // add the last event
    }
}

#endif
