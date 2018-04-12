/*############################################################################
  # Copyright 2016-2017 Intel Corporation
  #
  # Licensed under the Apache License, Version 2.0 (the "License");
  # you may not use this file except in compliance with the License.
  # You may obtain a copy of the License at
  #
  #     http://www.apache.org/licenses/LICENSE-2.0
  #
  # Unless required by applicable law or agreed to in writing, software
  # distributed under the License is distributed on an "AS IS" BASIS,
  # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  # See the License for the specific language governing permissions and
  # limitations under the License.
  ############################################################################*/

/*
//     Intel(R) Performance Primitives. Cryptography Primitives.
//     GF(p) methods
//
*/
#include "owndefs.h"
#include "owncp.h"

#include "pcpgfpmethod.h"
#include "pcpgfpstuff.h"

#if(_IPP >= _IPP_P8) || (_IPP32E >= _IPP32E_M7)

/* arithmetic over P-256r1 NIST modulus */
BNU_CHUNK_T* p256r1_add(BNU_CHUNK_T* res, const BNU_CHUNK_T* a, const BNU_CHUNK_T* b, IppsGFpState* pGF);
BNU_CHUNK_T* p256r1_sub(BNU_CHUNK_T* res, const BNU_CHUNK_T* a, const BNU_CHUNK_T* b, IppsGFpState* pGF);
BNU_CHUNK_T* p256r1_neg(BNU_CHUNK_T* res, const BNU_CHUNK_T* a, IppsGFpState* pGF);
BNU_CHUNK_T* p256r1_div_by_2 (BNU_CHUNK_T* res, const BNU_CHUNK_T* a, IppsGFpState* pGF);
BNU_CHUNK_T* p256r1_mul_by_2 (BNU_CHUNK_T* res, const BNU_CHUNK_T* a, IppsGFpState* pGF);
BNU_CHUNK_T* p256r1_mul_by_3 (BNU_CHUNK_T* res, const BNU_CHUNK_T* a, IppsGFpState* pGF);
#if(_IPP_ARCH ==_IPP_ARCH_EM64T)
BNU_CHUNK_T* p256r1_mul_montl(BNU_CHUNK_T* res, const BNU_CHUNK_T* a, const BNU_CHUNK_T* b, IppsGFpState* pGF);
BNU_CHUNK_T* p256r1_mul_montx(BNU_CHUNK_T* res, const BNU_CHUNK_T* a, const BNU_CHUNK_T* b, IppsGFpState* pGF);
BNU_CHUNK_T* p256r1_sqr_montl(BNU_CHUNK_T* res, const BNU_CHUNK_T* a, IppsGFpState* pGF);
BNU_CHUNK_T* p256r1_sqr_montx(BNU_CHUNK_T* res, const BNU_CHUNK_T* a, IppsGFpState* pGF);
BNU_CHUNK_T* p256r1_to_mont  (BNU_CHUNK_T* res, const BNU_CHUNK_T* a, IppsGFpState* pGF);
BNU_CHUNK_T* p256r1_mont_back(BNU_CHUNK_T* res, const BNU_CHUNK_T* a, IppsGFpState* pGF);
#endif
#if(_IPP_ARCH ==_IPP_ARCH_IA32)
BNU_CHUNK_T* p256r1_mul_mont_slm(BNU_CHUNK_T* res, const BNU_CHUNK_T* a, const BNU_CHUNK_T* b, IppsGFpState* pGF);
BNU_CHUNK_T* p256r1_sqr_mont_slm(BNU_CHUNK_T* res, const BNU_CHUNK_T* a, IppsGFpState* pGF);
BNU_CHUNK_T* p256r1_mred(BNU_CHUNK_T* res, BNU_CHUNK_T* product);
#endif

#define OPERAND_BITSIZE (256)
#define LEN_P256        (BITS_BNU_CHUNK(OPERAND_BITSIZE))


/*
// ia32 multiplicative methods
*/
#if (_IPP_ARCH ==_IPP_ARCH_IA32 )
static BNU_CHUNK_T* p256r1_mul_montl(BNU_CHUNK_T* pR, const BNU_CHUNK_T* pA, const BNU_CHUNK_T* pB, IppsGFpState* pGF)
{
   BNU_CHUNK_T* product = GFP_POOL(pGF);
   cpMulAdc_BNU_school(product, pA,LEN_P256, pB,LEN_P256);
   p256r1_mred(pR, product);
   return pR;
}

static BNU_CHUNK_T* p256r1_sqr_montl(BNU_CHUNK_T* pR, const BNU_CHUNK_T* pA, IppsGFpState* pGF)
{
   BNU_CHUNK_T* product = GFP_POOL(pGF);
   cpSqrAdc_BNU_school(product, pA,LEN_P256);
   p256r1_mred(pR, product);
   return pR;
}


/*
// Montgomery domain conversion constants
*/
static BNU_CHUNK_T RR[] = {
   0x00000003,0x00000000, 0xffffffff,0xfffffffb,
   0xfffffffe,0xffffffff, 0xfffffffd,0x00000004};

static BNU_CHUNK_T one[] = {
   1,0,0,0,0,0,0,0};

static BNU_CHUNK_T* p256r1_to_mont(BNU_CHUNK_T* pR, const BNU_CHUNK_T* pA, IppsGFpState* pGF)
{
   return p256r1_mul_montl(pR, pA, (BNU_CHUNK_T*)RR, pGF);
}

static BNU_CHUNK_T* p256r1_mont_back(BNU_CHUNK_T* pR, const BNU_CHUNK_T* pA, IppsGFpState* pGF)
{
   return p256r1_mul_montl(pR, pA, (BNU_CHUNK_T*)one, pGF);
}

static BNU_CHUNK_T* p256r1_to_mont_slm(BNU_CHUNK_T* pR, const BNU_CHUNK_T* pA, IppsGFpState* pGF)
{
   return p256r1_mul_mont_slm(pR, pA, (BNU_CHUNK_T*)RR, pGF);
}

static BNU_CHUNK_T* p256r1_mont_back_slm(BNU_CHUNK_T* pR, const BNU_CHUNK_T* pA, IppsGFpState* pGF)
{
   return p256r1_mul_mont_slm(pR, pA, (BNU_CHUNK_T*)one, pGF);
}
#endif

/*
// returns methods
*/
IPPFUN( const IppsGFpMethod*, ippsGFpMethod_p256r1, (void) )
{
   static IppsGFpMethod method = {
      p256r1_add,
      p256r1_sub,
      p256r1_neg,
      p256r1_div_by_2,
      p256r1_mul_by_2,
      p256r1_mul_by_3,
      p256r1_mul_montl,
      p256r1_sqr_montl,
      p256r1_to_mont,
      p256r1_mont_back
   };

   #if(_IPP_ARCH==_IPP_ARCH_EM64T) && ((_ADCOX_NI_ENABLING_==_FEATURE_ON_) || (_ADCOX_NI_ENABLING_==_FEATURE_TICKTOCK_))
   if(IsFeatureEnabled(ADCOX_ENABLED)) {
      method.mul = p256r1_mul_montx;
      method.sqr = p256r1_sqr_montx;
   }
   #endif

   #if(_IPP_ARCH==_IPP_ARCH_IA32)
   if(IsFeatureEnabled(ippCPUID_SSSE3|ippCPUID_MOVBE) && !IsFeatureEnabled(ippCPUID_AVX)) {
      method.mul = p256r1_mul_mont_slm;
      method.sqr = p256r1_sqr_mont_slm;
      method.encode = p256r1_to_mont_slm;
      method.decode = p256r1_mont_back_slm;
   }
   #endif

   return &method;
}

#undef LEN_P256
#undef OPERAND_BITSIZE


#else
IPPFUN( const IppsGFpMethod*, ippsGFpMethod_p256r1, (void) )
{
   return ippsGFpMethod_pArb();
}
#endif
