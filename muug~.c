/**

Copyright (C) 2006,2007,2008 Claude Heiland-Allen
Copyright (C) 2010 Paulo Casaes

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


-- 
http://http://gitorious.org/~saturno
mailto:irmaosaturno@gmail.com

http://claudiusmaximus.goto10.org
mailto:claudiusmaximus@goto10.org


*/

#include "m_pd.h"
#include <math.h>

#define muug_t double   //change to float to lower processing to single precision

struct muug_tilde {
  t_object pd;
  t_float dummy;
  muug_t v;
  muug_t ya1;
  muug_t wa1;
  muug_t yb1;
  muug_t wb1;
  muug_t yc1;
  muug_t wc1;
  muug_t yd1;
} t_sigrpole;

static t_class *muug_tilde_class;

#define MUUG_PI 3.14159265358979323846
#define MUUG_TILDE_TABLE_RANGE 4
#ifdef MUUG_TILDE_TABLE_SIZE
#else
#define MUUG_TILDE_TABLE_SIZE 512 //262144=2^18
#endif


#ifdef MUUG_INTERPOLATE
#else
#define MUUG_INTERPOLATE 0
#endif



static muug_t muug_tilde_tanh[MUUG_TILDE_TABLE_SIZE+1];

static inline muug_t ttanh(t_sample x) {
#if MUUG_INTERPOLATE == 0
  int t = rint(MUUG_TILDE_TABLE_SIZE * (x + MUUG_TILDE_TABLE_RANGE) / (MUUG_TILDE_TABLE_RANGE * 2));
  if (t < 0) t = 0;
  if (t > MUUG_TILDE_TABLE_SIZE) { 
		t = MUUG_TILDE_TABLE_SIZE;
	}
  return muug_tilde_tanh[t];
#else
	t_sample pos = MUUG_TILDE_TABLE_SIZE * (x + MUUG_TILDE_TABLE_RANGE) / (MUUG_TILDE_TABLE_RANGE * 2);
  int t = ceil(pos);
  int u = floor(pos);
  if (t < 0) t = 0;
  if (t > MUUG_TILDE_TABLE_SIZE) { 
		t = MUUG_TILDE_TABLE_SIZE;
	}
  if (u < 0) u = 0;
  if (u > MUUG_TILDE_TABLE_SIZE) { 
		u = MUUG_TILDE_TABLE_SIZE;
	}
	if(t != u) {
		return muug_tilde_tanh[u] + (pos -u) * ((muug_tilde_tanh[t]-muug_tilde_tanh[u])/(t-u));
	}
  return muug_tilde_tanh[t];
#endif
}

#define ttanh tanhf

static void *muug_tilde_new(void) {
  struct muug_tilde *muug = (struct muug_tilde *) pd_new(muug_tilde_class);
  inlet_new(&muug->pd, &muug->pd.ob_pd, &s_signal, &s_signal);
  inlet_new(&muug->pd, &muug->pd.ob_pd, &s_signal, &s_signal);
  outlet_new(&muug->pd, &s_signal);
  muug->v   = 2;
  muug->ya1 = 0;
  muug->wa1 = 0;
  muug->yb1 = 0;
  muug->wb1 = 0;
  muug->yc1 = 0;
  muug->wc1 = 0;
  muug->yd1 = 0;
	
  return muug;
}

static t_int *muug_tilde_perform(t_int *w) {
  t_signal *inputsig      = (t_signal *)         (w[1]);
  t_sample *in_audio      = inputsig->s_vec;
  t_sample *in_frequency  = (t_sample *)         (w[2]);
  t_sample *in_resonance  = (t_sample *)         (w[3]);
  t_sample *out_audio     = (t_sample *)         (w[4]);
  struct muug_tilde *muug = (struct muug_tilde *)(w[5]);
  int n                   = (t_int)              (w[6]);
  

  muug_t v   = muug->v;
  muug_t ya1 = muug->ya1;
  muug_t wa1 = muug->wa1;
  muug_t yb1 = muug->yb1;
  muug_t wb1 = muug->wb1;
  muug_t yc1 = muug->yc1;
  muug_t wc1 = muug->wc1;
  muug_t yd1 = muug->yd1;
  for (int i = 0; i < n; ++i) {
    t_sample x = *in_audio++;
    t_sample f = *in_frequency++;
    t_sample r = *in_resonance++;
    muug_t g = 1 - expf(-2 * MUUG_PI * f /inputsig->s_sr);
    muug_t ya = ya1 + v * g * ttanh((x - 4 * r * yd1) / v - wa1);
    muug_t wa = ttanh(ya / v);
    muug_t yb = yb1 + v * g * (wa - wb1);
    muug_t wb = ttanh(yb / v);
    muug_t yc = yc1 + v * g * (wb - wc1);
    muug_t wc = ttanh(yc / v);
    muug_t yd = yd1 + v * g * (wc - ttanh(yd1 / v));
    t_sample y = yd;
    ya1 = ya;
    wa1 = wa;
    yb1 = yb;
    wb1 = wb;
    yc1 = yc;
    wc1 = wc;
    yd1 = yd;
    *out_audio++ = y;
  }
  muug->ya1 = ya1;
  muug->wa1 = wa1;
  muug->yb1 = yb1;
  muug->wb1 = wb1;
  muug->yc1 = yc1;
  muug->wc1 = wc1;
  muug->yd1 = yd1;
  return w+7;
}

static void muug_tilde_dsp(struct muug_tilde *muug, t_signal **sp) {
  dsp_add(muug_tilde_perform, 6, sp[0], sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, muug, sp[0]->s_n);
}

extern void muug_tilde_setup(void) {

	post("muug~: loading tanh table");
  for (int t = 0; t <= MUUG_TILDE_TABLE_SIZE; ++t) {
    t_sample x = (MUUG_TILDE_TABLE_RANGE * 2) * ((t_sample) t) / MUUG_TILDE_TABLE_SIZE - MUUG_TILDE_TABLE_RANGE;
    muug_tilde_tanh[t] = tanh(x);
  }
  muug_tilde_class = class_new(gensym("muug~"), (t_newmethod) muug_tilde_new, 0, sizeof(struct muug_tilde), 0, 0, 0);
  CLASS_MAINSIGNALIN(muug_tilde_class, struct muug_tilde, dummy);
  class_addmethod(muug_tilde_class, (t_method) muug_tilde_dsp, gensym("dsp"), 0);
  

  
}
