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
//     GF(p^d) methods
//
*/
#include "owndefs.h"
#include "owncp.h"

#include "pcpgfpstuff.h"

#define cpGFpxAdd_com OWNAPI(cpGFpxAdd_com)
BNU_CHUNK_T* cpGFpxAdd_com(BNU_CHUNK_T* pR, const BNU_CHUNK_T* pA, const BNU_CHUNK_T* pB, IppsGFpState* pGFpx);
#define cpGFpxSub_com OWNAPI(cpGFpxSub_com)
BNU_CHUNK_T* cpGFpxSub_com(BNU_CHUNK_T* pR, const BNU_CHUNK_T* pA, const BNU_CHUNK_T* pB, IppsGFpState* pGFpx);
#define cpGFpxNeg_com OWNAPI(cpGFpxNeg_com)
BNU_CHUNK_T* cpGFpxNeg_com(BNU_CHUNK_T* pR, const BNU_CHUNK_T* pA, IppsGFpState* pGFpx);
#define cpGFpxMul_com OWNAPI(cpGFpxMul_com)
BNU_CHUNK_T* cpGFpxMul_com(BNU_CHUNK_T* pR, const BNU_CHUNK_T* pA, const BNU_CHUNK_T* pB, IppsGFpState* pGFpx);
#define cpGFpxSqr_com OWNAPI(cpGFpxSqr_com)
BNU_CHUNK_T* cpGFpxSqr_com(BNU_CHUNK_T* pR, const BNU_CHUNK_T* pA, IppsGFpState* pGFpx);
#define cpGFpxDiv2_com OWNAPI(cpGFpxDiv2_com)
BNU_CHUNK_T* cpGFpxDiv2_com(BNU_CHUNK_T* pR, const BNU_CHUNK_T* pA, IppsGFpState* pGFpx);
BNU_CHUNK_T* cpGFpxMul2_com(BNU_CHUNK_T* pR, const BNU_CHUNK_T* pA, IppsGFpState* pGFpx);
BNU_CHUNK_T* cpGFpxMul3_com(BNU_CHUNK_T* pR, const BNU_CHUNK_T* pA, IppsGFpState* pGFpx);
BNU_CHUNK_T* cpGFpxEncode_com(BNU_CHUNK_T* pR, const BNU_CHUNK_T* pA, IppsGFpState* pGFpx);
BNU_CHUNK_T* cpGFpxDecode_com(BNU_CHUNK_T* pR, const BNU_CHUNK_T* pA, IppsGFpState* pGFpx);
