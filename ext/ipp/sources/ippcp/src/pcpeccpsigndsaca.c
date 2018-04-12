/*############################################################################
  # Copyright 2003-2017 Intel Corporation
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
// 
//  Purpose:
//     Cryptography Primitive.
//     EC over Prime Finite Field (Sign, DSA version)
// 
//  Contents:
//     ippsECCPSignDSA()
// 
// 
*/

#include "owndefs.h"
#include "owncp.h"
#include "pcpeccp.h"


/*F*
//    Name: ippsECCPSignDSA
//
// Purpose: Signing of message representative.
//          (DSA version).
//
// Returns:                   Reason:
//    ippStsNullPtrErr           NULL == pECC
//                               NULL == pMsgDigest
//                               NULL == pPrivate
//                               NULL == pSignX
//                               NULL == pSignY
//
//    ippStsContextMatchErr      illegal pECC->idCtx
//                               illegal pMsgDigest->idCtx
//                               illegal pPrivate->idCtx
//                               illegal pSignX->idCtx
//                               illegal pSignY->idCtx
//
//    ippStsMessageErr           MsgDigest >= order
//
//    ippStsRangeErr             not enough room for:
//                               signX
//                               signY
//
//    ippStsEphemeralKeyErr      (0==signX) || (0==signY)
//
//    ippStsNoErr                no errors
//
// Parameters:
//    pMsgDigest     pointer to the message representative to be signed
//    pPrivate       pointer to the regular private key
//    pSignX,pSignY  pointer to the signature
//    pECC           pointer to the ECCP context
//
// Note:
//    - ephemeral key pair extracted from pECC and
//      must be generated and before ippsECCPDSASign() usage
//    - ephemeral key pair destroy before exit
//
*F*/
IPPFUN(IppStatus, ippsECCPSignDSA,(const IppsBigNumState* pMsgDigest,
                                   const IppsBigNumState* pPrivate,
                                   IppsBigNumState* pSignX, IppsBigNumState* pSignY,
                                   IppsECCPState* pEC))
{
   /* use aligned EC context */
   IPP_BAD_PTR1_RET(pEC);
   pEC = (IppsGFpECState*)( IPP_ALIGNED_PTR(pEC, ECGFP_ALIGNMENT) );
   IPP_BADARG_RET(!ECP_TEST_ID(pEC), ippStsContextMatchErr);

   /* test private key*/
   IPP_BAD_PTR1_RET(pPrivate);
   pPrivate = (IppsBigNumState*)( IPP_ALIGNED_PTR(pPrivate, BN_ALIGNMENT) );
   IPP_BADARG_RET(!BN_VALID_ID(pPrivate), ippStsContextMatchErr);

   /* test message representative */
   IPP_BAD_PTR1_RET(pMsgDigest);
   pMsgDigest = (IppsBigNumState*)( IPP_ALIGNED_PTR(pMsgDigest, BN_ALIGNMENT) );
   IPP_BADARG_RET(!BN_VALID_ID(pMsgDigest), ippStsContextMatchErr);
   IPP_BADARG_RET(BN_NEGATIVE(pMsgDigest), ippStsMessageErr);

   /* test signature */
   IPP_BAD_PTR2_RET(pSignX,pSignY);
   pSignX = (IppsBigNumState*)( IPP_ALIGNED_PTR(pSignX, BN_ALIGNMENT) );
   pSignY = (IppsBigNumState*)( IPP_ALIGNED_PTR(pSignY, BN_ALIGNMENT) );
   IPP_BADARG_RET(!BN_VALID_ID(pSignX), ippStsContextMatchErr);
   IPP_BADARG_RET(!BN_VALID_ID(pSignY), ippStsContextMatchErr);
   IPP_BADARG_RET((BN_ROOM(pSignX)*BITSIZE(BNU_CHUNK_T)<ECP_ORDBITSIZE(pEC)), ippStsRangeErr);
   IPP_BADARG_RET((BN_ROOM(pSignY)*BITSIZE(BNU_CHUNK_T)<ECP_ORDBITSIZE(pEC)), ippStsRangeErr);

   {
      IppsMontState* pMontR = ECP_MONT_R(pEC);
      BNU_CHUNK_T* pOrder = MNT_MODULUS(pMontR);
      int orderLen = MNT_SIZE(pMontR);

      BNU_CHUNK_T* pMsgData = BN_NUMBER(pMsgDigest);
      int msgLen = BN_SIZE(pMsgDigest);
      IPP_BADARG_RET(0<=cpCmp_BNU(pMsgData, msgLen, pOrder, orderLen), ippStsMessageErr);

      /* signY = ephPrivate^-1 mod Order*/
      {
         __ALIGN8 IppsBigNumState R;
         BNU_CHUNK_T* buffer = ECP_SBUFFER(pEC);
         /* BN(order) */
         BN_Make(buffer, buffer+orderLen+1, orderLen, &R);
         BN_Set(pOrder, orderLen, &R);

         BN_Set(ECP_PRIVAT_E(pEC), orderLen, pSignX);
         ippsModInv_BN(pSignX, &R, pSignY);
      }

      {
         IppStatus sts = ippStsEphemeralKeyErr;

         IppsGFpState* pGF = ECP_GFP(pEC);
         int elmLen = GFP_FELEN(pGF);
         int pelmLen = GFP_PELEN(pGF);
         BNU_CHUNK_T* pC = cpGFpGetPool(3, pGF);
         BNU_CHUNK_T* pF = pC + pelmLen;
         BNU_CHUNK_T* pS = pF + pelmLen;

         /* ephemeral public */
         IppsGFpECPoint  ephPublic;
         cpEcGFpInitPoint(&ephPublic, ECP_PUBLIC_E(pEC), ECP_FINITE_POINT|ECP_AFFINE_POINT, pEC);

         /* ephPublic.x  */
         gfec_GetPoint(pC, NULL, &ephPublic, pEC);
         pGF->decode(pC, pC, pGF);

         /* signX = int(ephPublic.x) (mod order) */
         elmLen = cpMod_BNU(pC, elmLen, pOrder, orderLen);
         cpGFpElementPadd(pC+elmLen, orderLen-elmLen, 0);
         if(!GFP_IS_ZERO(pC, orderLen)) {

            /*
            // signY = (1/ephPrivate)*(pMsgDigest + private*signX) (mod order)
            */
            /* S = mont(private) * mont(signX) */
            cpMontEnc_BNU(pF, pC, orderLen, pMontR);
            cpMontEnc_BNU(pS, BN_NUMBER(pPrivate), BN_SIZE(pPrivate), pMontR);
            cpMontMul_BNU(pS, pS,orderLen, pF,orderLen,
                          pOrder,orderLen,
                          MNT_HELPER(pMontR), MNT_PRODUCT(pMontR), NULL);
            /* S = (S+ mont(msg) mod order */
            cpGFpElementCopyPadd(pF, orderLen, pMsgData, msgLen);
            cpMontEnc_BNU(pF, pF, orderLen, pMontR);
            cpModAdd_BNU(pS, pS, pF, pOrder, orderLen, pF);

            /* S = S*(ephPrivate^-1) */
            cpGFpElementCopyPadd(pF, orderLen, BN_NUMBER(pSignY), BN_SIZE(pSignY));
            cpMontMul_BNU(pS, pS,orderLen, pF,orderLen,
                          pOrder,orderLen,
                          MNT_HELPER(pMontR), MNT_PRODUCT(pMontR), NULL);

            if(!GFP_IS_ZERO(pS, orderLen)) {
               BNU_CHUNK_T* pSignXdata = BN_NUMBER(pSignX);
               BNU_CHUNK_T* pSignYdata = BN_NUMBER(pSignY);

               /* signX */
               elmLen = orderLen;
               FIX_BNU(pC, elmLen);
               BN_SIGN(pSignX) = ippBigNumPOS;
               BN_SIZE(pSignX) = elmLen;
               cpGFpElementCopy(pSignXdata, pC, elmLen);

               /* signY */
               elmLen = orderLen;
               FIX_BNU(pS, elmLen);
               BN_SIGN(pSignY) = ippBigNumPOS;
               BN_SIZE(pSignY) = elmLen;
               cpGFpElementCopy(pSignYdata, pS, elmLen);

               sts = ippStsNoErr;
            }
         }

         cpGFpReleasePool(3, pGF);
         return sts;
      }
   }
}
