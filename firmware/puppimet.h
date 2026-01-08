#ifndef PUPPIMET_H
#define PUPPIMET_H

#include "data.h"
#define POLY_MAP 0.004363323129985824 // pi / 720


void Sum_Particles(Particle_xy proj_xy[N_INPUT_LINKS],
                   Particle_xy &slice_sum);


void puppimet_xy(Particle_T in_particles[N_INPUT_LINKS], 
                 Particle_xy &met_xy, 
                 METCtrlToken token_d, 
                 METCtrlToken& token_q,
                 METCtrlToken token_i);

void pxpy_to_ptphi(const Particle_xy met_xy, Sum &out_met, METCtrlToken token_d, METCtrlToken& token_q);

#endif