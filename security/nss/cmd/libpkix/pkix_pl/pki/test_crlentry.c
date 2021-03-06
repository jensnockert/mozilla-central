/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the PKIX-C library.
 *
 * The Initial Developer of the Original Code is
 * Sun Microsystems, Inc.
 * Portions created by the Initial Developer are
 * Copyright 2004-2007 Sun Microsystems, Inc.  All Rights Reserved.
 *
 * Contributor(s):
 *   Sun Microsystems, Inc.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
/*
 * test_crlentry.c
 *
 * Test CRLENTRY Type
 *
 */

#include "testutil.h"
#include "testutil_nss.h"

static void *plContext = NULL;

static
void createCRLEntries(
        char *dataDir,
        char *crlInput,
        PKIX_PL_CRL **pCrl,
        PKIX_PL_CRLEntry **goodObject,
        PKIX_PL_CRLEntry **equalObject,
        PKIX_PL_CRLEntry **diffObject)
{
        PKIX_PL_CRL *crl = NULL;
        PKIX_PL_BigInt *firstSNBigInt = NULL;
        PKIX_PL_BigInt *secondSNBigInt = NULL;
        PKIX_PL_String *firstSNString = NULL;
        PKIX_PL_String *secondSNString = NULL;
        char *firstSNAscii = "010932";
        char *secondSNAscii = "3039";

        PKIX_TEST_STD_VARS();

        subTest("PKIX_PL_CRL_Create <crl>");
        crl = createCRL(dataDir, crlInput, plContext);

        subTest("PKIX_PL_CRL_GetCRLEntryForSerialNumber");
        PKIX_TEST_EXPECT_NO_ERROR(PKIX_PL_String_Create(
                                    PKIX_ESCASCII,
                                    firstSNAscii,
                                    PL_strlen(firstSNAscii),
                                    &firstSNString,
                                    plContext));

        PKIX_TEST_EXPECT_NO_ERROR(PKIX_PL_BigInt_Create(
                            firstSNString,
                            &firstSNBigInt,
                            plContext));

        PKIX_TEST_EXPECT_NO_ERROR(PKIX_PL_CRL_GetCRLEntryForSerialNumber(
                            crl, firstSNBigInt, goodObject, plContext));

        PKIX_TEST_EXPECT_NO_ERROR(PKIX_PL_CRL_GetCRLEntryForSerialNumber(
                            crl, firstSNBigInt, equalObject, plContext));

        PKIX_TEST_EXPECT_NO_ERROR(PKIX_PL_String_Create(
                                    PKIX_ESCASCII,
                                    secondSNAscii,
                                    PL_strlen(secondSNAscii),
                                    &secondSNString,
                                    plContext));

        PKIX_TEST_EXPECT_NO_ERROR(PKIX_PL_BigInt_Create(
                            secondSNString,
                            &secondSNBigInt,
                            plContext));

        PKIX_TEST_EXPECT_NO_ERROR(PKIX_PL_CRL_GetCRLEntryForSerialNumber(
                            crl, secondSNBigInt, diffObject, plContext));



        *pCrl = crl;

cleanup:
        PKIX_TEST_DECREF_AC(firstSNBigInt);
        PKIX_TEST_DECREF_AC(secondSNBigInt);
        PKIX_TEST_DECREF_AC(firstSNString);
        PKIX_TEST_DECREF_AC(secondSNString);
        PKIX_TEST_RETURN();
}

static void testGetReasonCode(
        PKIX_PL_CRLEntry *goodObject)
{
        PKIX_Int32 reasonCode = 0;
        PKIX_Int32 expectedReasonCode = 260;

        PKIX_TEST_STD_VARS();

        subTest("PKIX_PL_CRLEntry_GetCRLEntryReasonCode");

        PKIX_TEST_EXPECT_NO_ERROR(PKIX_PL_CRLEntry_GetCRLEntryReasonCode(
                            goodObject, &reasonCode, plContext));

        if (reasonCode != expectedReasonCode) {
                testError("unexpected value of CRL Entry Reason Code");
                (void) printf("Actual value:\t%d\n", reasonCode);
                (void) printf("Expected value:\t%d\n", expectedReasonCode);
                goto cleanup;
        }

cleanup:
        PKIX_TEST_RETURN();
}

static void
testCritExtensionsAbsent(PKIX_PL_CRLEntry *crlEntry)
{
        PKIX_List *oidList = NULL;
        PKIX_UInt32 numOids = 0;

        PKIX_TEST_STD_VARS();

        PKIX_TEST_EXPECT_NO_ERROR(PKIX_PL_CRLEntry_GetCriticalExtensionOIDs
                                    (crlEntry, &oidList, plContext));

        PKIX_TEST_EXPECT_NO_ERROR(PKIX_List_GetLength
                                    (oidList, &numOids, plContext));
        if (numOids != 0){
                pkixTestErrorMsg = "unexpected mismatch";
        }

cleanup:

        PKIX_TEST_DECREF_AC(oidList);

        PKIX_TEST_RETURN();
}

static void
testGetCriticalExtensionOIDs(PKIX_PL_CRLEntry *goodObject)
{
        subTest("PKIX_PL_CRL_GetCriticalExtensionOIDs "
                "<CritExtensionsAbsent>");
        testCritExtensionsAbsent(goodObject);

}

static
void printUsage(void) {
        (void) printf("\nUSAGE:\ttest_crlentry <data-dir>\n\n");
}

/* Functional tests for CRLENTRY public functions */

int test_crlentry(int argc, char *argv[]) {
        PKIX_PL_CRL *crl = NULL;
        PKIX_PL_CRLEntry *goodObject = NULL;
        PKIX_PL_CRLEntry *equalObject = NULL;
        PKIX_PL_CRLEntry *diffObject = NULL;
        PKIX_UInt32 actualMinorVersion;
        PKIX_UInt32 j = 0;

        char *dataDir = NULL;
        char *goodInput = "crlgood.crl";
        char *expectedAscii =
                "\n\t[\n"
                "\tSerialNumber:    010932\n"
                "\tReasonCode:      260\n"
                "\tRevocationDate:  Fri Jan 07 15:09:10 2005\n"
                "\tCritExtOIDs:     (EMPTY)\n"
                "\t]\n\t";

        /* Note XXX serialnumber and reasoncode need debug */

        PKIX_TEST_STD_VARS();

        startTests("CRLEntry");

        PKIX_TEST_EXPECT_NO_ERROR(
            PKIX_PL_NssContext_Create(0, PKIX_FALSE, NULL, &plContext));

        if (argc < 1+j) {
                printUsage();
                return (0);
        }

        dataDir = argv[1+j];

        createCRLEntries
            (dataDir, goodInput, &crl, &goodObject, &equalObject, &diffObject);

        PKIX_TEST_EQ_HASH_TOSTR_DUP
                (goodObject,
                equalObject,
                diffObject,
                NULL, /* expectedAscii, */
                CRLENTRY,
                PKIX_TRUE);

        testGetReasonCode(goodObject);

        testGetCriticalExtensionOIDs(goodObject);

cleanup:

        PKIX_TEST_DECREF_AC(crl);
        PKIX_TEST_DECREF_AC(goodObject);
        PKIX_TEST_DECREF_AC(equalObject);
        PKIX_TEST_DECREF_AC(diffObject);

        PKIX_Shutdown(plContext);

        PKIX_TEST_RETURN();

        endTests("CRLEntry");

        return (0);
}
