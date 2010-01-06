/**
 * \file   sincos.c
 * \brief  Provide a sincos and sincosf implementation, used implicitly by gcc
 *
 * \date   2008-05-27
 * \author Adam Lackorzynski <adam@os.inf.tu-dresden.de>
 *
 */
/*
 * (c) 2008-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */

#include <math.h>

void sincos(double x, double *s, double *c);
void sincosf(float x, float *s, float *c);

void sincos(double x, double *s, double *c)
{
  *s = sin(x);
  *c = cos(x);
}

void sincosf(float x, float *s, float *c)
{
  *s = sinf(x);
  *c = cosf(x);
}
