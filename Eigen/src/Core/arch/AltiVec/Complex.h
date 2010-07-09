// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2010 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// Eigen. If not, see <http://www.gnu.org/licenses/>.

#ifndef EIGEN_COMPLEX_ALTIVEC_H
#define EIGEN_COMPLEX_ALTIVEC_H

static Packet4ui  ei_p4ui_CONJ_XOR = vec_mergeh((Packet4ui)ei_p4i_ZERO, (Packet4ui)ei_p4f_ZERO_);//{ 0x00000000, 0x80000000, 0x00000000, 0x80000000 };
static Packet16uc ei_p16uc_COMPLEX_RE   = vec_sld((Packet16uc) vec_splat((Packet4ui)ei_p16uc_FORWARD, 0), (Packet16uc) vec_splat((Packet4ui)ei_p16uc_FORWARD, 2), 8);//{ 0,1,2,3, 0,1,2,3, 8,9,10,11, 8,9,10,11 };
static Packet16uc ei_p16uc_COMPLEX_IM   = vec_sld((Packet16uc) vec_splat((Packet4ui)ei_p16uc_FORWARD, 1), (Packet16uc) vec_splat((Packet4ui)ei_p16uc_FORWARD, 3), 8);//{ 4,5,6,7, 4,5,6,7, 12,13,14,15, 12,13,14,15 };
static Packet16uc ei_p16uc_COMPLEX_REV  = vec_sld(ei_p16uc_REVERSE, ei_p16uc_REVERSE, 8);//{ 4,5,6,7, 0,1,2,3, 12,13,14,15, 8,9,10,11 };
static Packet16uc ei_p16uc_COMPLEX_REV2 = vec_sld(ei_p16uc_FORWARD, ei_p16uc_FORWARD, 8);//{ 8,9,10,11, 12,13,14,15, 0,1,2,3, 4,5,6,7 };
static Packet16uc ei_p16uc_PSET_HI = (Packet16uc) vec_mergeh((Packet4ui) vec_splat((Packet4ui)ei_p16uc_FORWARD, 0), (Packet4ui) vec_splat((Packet4ui)ei_p16uc_FORWARD, 1));//{ 0,1,2,3, 4,5,6,7, 0,1,2,3, 4,5,6,7 };
static Packet16uc ei_p16uc_PSET_LO = (Packet16uc) vec_mergeh((Packet4ui) vec_splat((Packet4ui)ei_p16uc_FORWARD, 2), (Packet4ui) vec_splat((Packet4ui)ei_p16uc_FORWARD, 3));//{ 8,9,10,11, 12,13,14,15, 8,9,10,11, 12,13,14,15 };

//---------- float ----------
struct Packet2cf
{
  EIGEN_STRONG_INLINE Packet2cf() {}
  EIGEN_STRONG_INLINE explicit Packet2cf(const Packet4f& a) : v(a) {}
  Packet4f  v;
};

template<> struct ei_packet_traits<std::complex<float> >  : ei_default_packet_traits
{
  typedef Packet2cf type;
  enum {
    Vectorizable = 1,
    size = 2,

    HasAdd    = 1,
    HasSub    = 1,
    HasMul    = 1,
    HasDiv    = 1,
    HasNegate = 1,
    HasAbs    = 0,
    HasAbs2   = 0,
    HasMin    = 0,
    HasMax    = 0,
    HasSetLinear = 0
  };
};

template<> struct ei_unpacket_traits<Packet2cf> { typedef std::complex<float> type; enum {size=2}; };

template<> EIGEN_STRONG_INLINE Packet2cf ei_pset1<std::complex<float> >(const std::complex<float>&  from)
{
  Packet2cf res;
  /* On AltiVec we cannot load 64-bit registers, so wa have to take care of alignment */
  if ((ptrdiff_t)&from % 16 == 0) {
    res.v = ei_pload((const float *)&from);
    res.v = vec_perm(res.v, res.v, ei_p16uc_PSET_HI);
  } else {
    res.v = ei_ploadu((const float *)&from);
    res.v = vec_perm(res.v, res.v, ei_p16uc_PSET_LO);
  }
  return res;
}

template<> EIGEN_STRONG_INLINE Packet2cf ei_padd<Packet2cf>(const Packet2cf& a, const Packet2cf& b) { return Packet2cf(vec_add(a.v,b.v)); }
template<> EIGEN_STRONG_INLINE Packet2cf ei_psub<Packet2cf>(const Packet2cf& a, const Packet2cf& b) { return Packet2cf(vec_sub(a.v,b.v)); }
template<> EIGEN_STRONG_INLINE Packet2cf ei_pnegate(const Packet2cf& a) { return Packet2cf(ei_psub<Packet4f>(ei_p4f_ZERO, a.v)); }
template<> EIGEN_STRONG_INLINE Packet2cf ei_pconj(const Packet2cf& a) { return Packet2cf((Packet4f)vec_xor((Packet4ui)a.v, ei_p4ui_CONJ_XOR)); }

template<> EIGEN_STRONG_INLINE Packet2cf ei_pmul<Packet2cf>(const Packet2cf& a, const Packet2cf& b)
{
  Packet4f v1, v2;

  // Permute and multiply the real parts of a and b
  v1 = vec_perm(a.v, a.v, ei_p16uc_COMPLEX_RE);
  // Get the imaginary parts of a
  v2 = vec_perm(a.v, a.v, ei_p16uc_COMPLEX_IM);
  // multiply a_re * b 
  v1 = vec_madd(v1, b.v, ei_p4f_ZERO);
  // multiply a_im * b and get the conjugate result
  v2 = vec_madd(v2, b.v, ei_p4f_ZERO);
  v2 = (Packet4f) vec_xor((Packet4ui)v2, ei_p4ui_CONJ_XOR);
  // permute back to a proper order
  v2 = vec_perm(v2, v2, ei_p16uc_COMPLEX_REV);
  
  return Packet2cf(vec_add(v1, v2));
}

template<> EIGEN_STRONG_INLINE Packet2cf ei_pand   <Packet2cf>(const Packet2cf& a, const Packet2cf& b) { return Packet2cf(vec_and(a.v,b.v)); }
template<> EIGEN_STRONG_INLINE Packet2cf ei_por    <Packet2cf>(const Packet2cf& a, const Packet2cf& b) { return Packet2cf(vec_or(a.v,b.v)); }
template<> EIGEN_STRONG_INLINE Packet2cf ei_pxor   <Packet2cf>(const Packet2cf& a, const Packet2cf& b) { return Packet2cf(vec_xor(a.v,b.v)); }
template<> EIGEN_STRONG_INLINE Packet2cf ei_pandnot<Packet2cf>(const Packet2cf& a, const Packet2cf& b) { return Packet2cf(vec_and(a.v, vec_nor(b.v,b.v))); }

template<> EIGEN_STRONG_INLINE Packet2cf ei_pload <std::complex<float> >(const std::complex<float>* from) { EIGEN_DEBUG_ALIGNED_LOAD return Packet2cf(ei_pload((const float*)from)); }
template<> EIGEN_STRONG_INLINE Packet2cf ei_ploadu<std::complex<float> >(const std::complex<float>* from) { EIGEN_DEBUG_UNALIGNED_LOAD return Packet2cf(ei_ploadu((const float*)from)); }

template<> EIGEN_STRONG_INLINE void ei_pstore <std::complex<float> >(std::complex<float> *   to, const Packet2cf& from) { EIGEN_DEBUG_ALIGNED_STORE ei_pstore((float*)to, from.v); }
template<> EIGEN_STRONG_INLINE void ei_pstoreu<std::complex<float> >(std::complex<float> *   to, const Packet2cf& from) { EIGEN_DEBUG_UNALIGNED_STORE ei_pstoreu((float*)to, from.v); }

template<> EIGEN_STRONG_INLINE void ei_prefetch<std::complex<float> >(const std::complex<float> *   addr) { vec_dstt((float *)addr, DST_CTRL(2,2,32), DST_CHAN); }

template<> EIGEN_STRONG_INLINE std::complex<float>  ei_pfirst<Packet2cf>(const Packet2cf& a)
{
  std::complex<float> EIGEN_ALIGN16 res[2];
  ei_pstore((float *)&res, a.v);

  return res[0];
}

template<> EIGEN_STRONG_INLINE Packet2cf ei_preverse(const Packet2cf& a)
{
  Packet4f rev_a;
  rev_a = vec_perm(a.v, a.v, ei_p16uc_COMPLEX_REV2);
  return Packet2cf(rev_a);
}

template<> EIGEN_STRONG_INLINE std::complex<float> ei_predux<Packet2cf>(const Packet2cf& a)
{
  Packet4f b;
  b = (Packet4f) vec_sld(a.v, a.v, 8);
  b = ei_padd(a.v, b);
  return ei_pfirst(Packet2cf(sum));
}

template<> EIGEN_STRONG_INLINE Packet2cf ei_preduxp<Packet2cf>(const Packet2cf* vecs)
{
  Packet4f b1, b2;
  
  b1 = (Packet4f) vec_sld(vecs[0].v, vecs[1].v, 8);
  b2 = (Packet4f) vec_sld(vecs[1].v, vecs[0].v, 8);
  b2 = (Packet4f) vec_sld(b2, b2, 8);
  b2 = ei_padd(b1, b2);

  return Packet2cf(b2);
}

template<> EIGEN_STRONG_INLINE std::complex<float> ei_predux_mul<Packet2cf>(const Packet2cf& a)
{
  Packet4f b;
  Packet2cf prod;
  b = (Packet4f) vec_sld(a.v, a.v, 8);
  prod = ei_pmul(a, Packet2cf(b));

  return ei_pfirst(prod);
}

template<int Offset>
struct ei_palign_impl<Offset,Packet2cf>
{
  EIGEN_STRONG_INLINE static void run(Packet2cf& first, const Packet2cf& second)
  {
    if (Offset==1)
    {
      first.v = vec_sld(first.v, second.v, 8);
    }
  }
};

template<> struct ei_conj_helper<Packet2cf, Packet2cf, false,true>
{
  EIGEN_STRONG_INLINE Packet2cf pmadd(const Packet2cf& x, const Packet2cf& y, const Packet2cf& c) const
  { return ei_padd(pmul(x,y),c); }

  EIGEN_STRONG_INLINE Packet2cf pmul(const Packet2cf& a, const Packet2cf& b) const
  {
    return ei_pmul(a, ei_pconj(b));
  }
};

template<> struct ei_conj_helper<Packet2cf, Packet2cf, true,false>
{
  EIGEN_STRONG_INLINE Packet2cf pmadd(const Packet2cf& x, const Packet2cf& y, const Packet2cf& c) const
  { return ei_padd(pmul(x,y),c); }

  EIGEN_STRONG_INLINE Packet2cf pmul(const Packet2cf& a, const Packet2cf& b) const
  {
    return ei_pmul(ei_pconj(a), b);
  }
};

template<> struct ei_conj_helper<Packet2cf, Packet2cf, true,true>
{
  EIGEN_STRONG_INLINE Packet2cf pmadd(const Packet2cf& x, const Packet2cf& y, const Packet2cf& c) const
  { return ei_padd(pmul(x,y),c); }

  EIGEN_STRONG_INLINE Packet2cf pmul(const Packet2cf& a, const Packet2cf& b) const
  {
    return ei_pconj(ei_pmul(a, b));
  }
};

template<> EIGEN_STRONG_INLINE Packet2cf ei_pdiv<Packet2cf>(const Packet2cf& a, const Packet2cf& b)
{
  // TODO optimize it for AltiVec
  Packet2cf res = ei_conj_helper<Packet2cf,Packet2cf,false,true>().pmul(a,b);
  Packet4f s = vec_madd(b.v, b.v, ei_p4f_ZERO);
  return Packet2cf(ei_pdiv(res.v, vec_add(s,vec_perm(s, s, ei_p16uc_COMPLEX_REV))));
}

#endif // EIGEN_COMPLEX_ALTIVEC_H
