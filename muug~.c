#include "m_pd.h"
#include <math.h>

struct muug_tilde {
  t_object pd;
  t_float dummy;
  t_sample sr;
  t_sample v;
  t_sample ya1;
  t_sample wa1;
  t_sample yb1;
  t_sample wb1;
  t_sample yc1;
  t_sample wc1;
  t_sample yd1;
} t_sigrpole;

static t_class *muug_tilde_class;

#define MUUG_TILDE_TABLE_RANGE 4
#define MUUG_TILDE_TABLE_SIZE 512
static t_sample muug_tilde_tanh[MUUG_TILDE_TABLE_SIZE];

static inline t_sample ttanh(t_sample x) {
  int t = MUUG_TILDE_TABLE_SIZE * (x + MUUG_TILDE_TABLE_RANGE) / (MUUG_TILDE_TABLE_RANGE * 2);
  if (t < 0) t = 0;
  if (t >= MUUG_TILDE_TABLE_SIZE) t = MUUG_TILDE_TABLE_SIZE - 1;
  return muug_tilde_tanh[t];
}

#define ttanh tanhf

static void *muug_tilde_new(void) {
  struct muug_tilde *muug = (struct muug_tilde *) pd_new(muug_tilde_class);
  inlet_new(&muug->pd, &muug->pd.ob_pd, &s_signal, &s_signal);
  inlet_new(&muug->pd, &muug->pd.ob_pd, &s_signal, &s_signal);
  outlet_new(&muug->pd, &s_signal);
  muug->sr  = 44100; // FIXME
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
  t_sample *in_audio      = (t_sample *)         (w[1]);
  t_sample *in_frequency  = (t_sample *)         (w[2]);
  t_sample *in_resonance  = (t_sample *)         (w[3]);
  t_sample *out_audio     = (t_sample *)         (w[4]);
  struct muug_tilde *muug = (struct muug_tilde *)(w[5]);
  int n                   = (t_int)              (w[6]);
  t_sample pi  = 3.141592654;
  t_sample v   = muug->v;
  t_sample ya1 = muug->ya1;
  t_sample wa1 = muug->wa1;
  t_sample yb1 = muug->yb1;
  t_sample wb1 = muug->wb1;
  t_sample yc1 = muug->yc1;
  t_sample wc1 = muug->wc1;
  t_sample yd1 = muug->yd1;
  for (int i = 0; i < n; ++i) {
    t_sample x = *in_audio++;
    t_sample f = *in_frequency++;
    t_sample r = *in_resonance++;
    t_sample g = 1 - expf(-2 * pi * f / muug->sr);
    t_sample ya = ya1 + v * g * ttanh((x - 4 * r * yd1) / v - wa1);
    t_sample wa = ttanh(ya / v);
    t_sample yb = yb1 + v * g * (wa - wb1);
    t_sample wb = ttanh(yb / v);
    t_sample yc = yc1 + v * g * (wb - wc1);
    t_sample wc = ttanh(yc / v);
    t_sample yd = yd1 + v * g * (wc - ttanh(yd1 / v));
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
  dsp_add(muug_tilde_perform, 6, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, muug, sp[0]->s_n);
}

extern void muug_tilde_setup(void) {
  for (int t = 0; t < MUUG_TILDE_TABLE_SIZE; ++t) {
    t_sample x = (MUUG_TILDE_TABLE_RANGE * 2) * ((t_sample) t) / MUUG_TILDE_TABLE_SIZE - MUUG_TILDE_TABLE_RANGE;
    muug_tilde_tanh[t] = tanhf(x);
  }
  muug_tilde_class = class_new(gensym("muug~"), (t_newmethod) muug_tilde_new, 0, sizeof(struct muug_tilde), 0, 0, 0);
  CLASS_MAINSIGNALIN(muug_tilde_class, struct muug_tilde, dummy);
  class_addmethod(muug_tilde_class, (t_method) muug_tilde_dsp, gensym("dsp"), 0);
}
