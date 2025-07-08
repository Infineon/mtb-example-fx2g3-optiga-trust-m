/***************************************************************************//**
* \file optiga_app.c
*
* \version 1.0.0
*
* \details  This file provides the code for generating a keypair, signing a
*           digest, and verifying the signature using the Optiga module, from an FX2G3
*           device.
*
* See \ref README.md ["README"]
*
*******************************************************************************
* \copyright
* The MIT License
*
* Copyright (c) 2021 Infineon Technologies AG
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE
*******************************************************************************/

/* Includes */
#include "optiga_app.h"
#include "cy_debug.h"
#include "pal_os_memory.h"
#include "pal_os_timer.h"

/* This variable is updated based on asynchronous Optiga operations */
static volatile optiga_lib_status_t optiga_lib_status;

/**
 * \name optiga_lib_callback
 * \brief Callback when optiga_lib_xxxx operation is completed asynchronously
 * \param context
 * \param return_status
 * \retval None
 */
// lint --e{818} suppress "argument "context" is not used in the sample provided"
static void optiga_lib_callback(void *context, optiga_lib_status_t return_status) {
    optiga_lib_status = return_status;
    if (NULL != context) {
        // callback to upper layer here
    }
}

/**
 * \name optiga_crypt_callback
 * \brief Callback when optiga_crypt_xxxx operation is completed asynchronously
 * \param context
 * \param return_status
 * \retval None
 */
//lint --e{818} suppress "argument "context" is not used in the sample provided"
static void optiga_crypt_callback(void * context, optiga_lib_status_t return_status)
{
    optiga_lib_status = return_status;
    if (NULL != context)
    {
        // callback to upper layer here
    }
}

/**
 * \name optiga_util_callback
 * \brief Callback when optiga_util_xxxx operation is completed asynchronously
 * \param context
 * \param return_status
 * \retval None
 */
//lint --e{818} suppress "argument "context" is not used in the sample provided"
static void optiga_util_callback(void * context, optiga_lib_status_t return_status)
{
    optiga_lib_status = return_status;
    if (NULL != context)
    {
        // callback to upper layer here
    }
}

/* Used to manage application on the Optiga */
optiga_util_t * me_util_instance = NULL;

/**
 * \name Cy_Optiga_Init
 * \brief Initialize the Optiga module
 * \retval None
 */
void Cy_Optiga_Init(void) {
    optiga_lib_status_t return_status = !OPTIGA_LIB_SUCCESS;
    pal_init();
    do {
        if (NULL == me_util_instance) {
            /* Create an instance of optiga_util to open the application on OPTIGA. */
            /* arg1 OPTIGA_INSTANCE_ID_0 replaced with direct value 0 */
            me_util_instance = optiga_util_create(0, optiga_lib_callback, NULL);
            if (NULL == me_util_instance) {
                OPTIGA_LOG_ERROR("Util Instance Could NOT be Created!\r\n");
                break;
            }
        }
        
        /**
         * Open the application on OPTIGA which is a precondition to perform any other operations
         * using optiga_util_open_application
         */
         optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_open_application(me_util_instance, 0);
        WAIT_AND_CHECK_STATUS(return_status, optiga_lib_status);
        OPTIGA_LOG_MESSAGE("Util Application Opened");

    }while(FALSE);
     OPTIGA_LOG_STATUS(__FUNCTION__, return_status);
}

/**
 * \name Cy_Optiga_Init
 * \brief De-initialize the Optiga module
 * \retval None
 */
void Cy_Optiga_Deinit(void) {
    optiga_lib_status_t return_status = !OPTIGA_LIB_SUCCESS;
    do {
        /**
         * Close the application on OPTIGA after all the operations are executed
         * using optiga_util_close_application
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_util_close_application(me_util_instance, 0);

        WAIT_AND_CHECK_STATUS(return_status, optiga_lib_status);

        /* destroy util and crypt instances */
        // lint --e{534} suppress "Error handling is not required so return value is not checked"
        optiga_util_destroy(me_util_instance);
        me_util_instance = NULL;
    } while (FALSE);
    pal_deinit();
    OPTIGA_LOG_STATUS(__FUNCTION__, return_status);
}

/**
 * \name printHex
 * \brief Inserts leading zero to visually adjust padding in logs, and prints the hex number
 * \param hexnum
 * \retval None
 */
void printHex(uint8_t hexnum){
	hexnum<=0xF ? Cy_Debug_AddToLog(1, "0x0%x ", hexnum) : Cy_Debug_AddToLog(1, "0x%x ", hexnum);
#if USBFS_LOGS_ENABLE
    vTaskDelay(10);
#endif
}

/**
 * \name printArray16
 * \brief Print an array with upto 16B per row
 * \param arrayName A name to use for the array in the print log
 * \param array
 * \param length in bytes
 * \param header Log upto 15 starting array elements in a row, before standard rows of 16B begin
 * \retval None
 */
void printArray16(char *arrayName, uint8_t *array, uint32_t length, bool header){
	uint32_t rem = length%16;
	char *ts = "          ";
	Cy_Debug_AddToLog(1, "%s%s:\r\n%s", ts, arrayName, ts);
#if USBFS_LOGS_ENABLE
    vTaskDelay(100);
#endif
	int i=0;
	while(i<rem && header) printHex(array[i++]);
#if USBFS_LOGS_ENABLE
    vTaskDelay(100);
#endif
	if(rem && header) Cy_Debug_AddToLog(1, "\r\n%s", ts);
#if USBFS_LOGS_ENABLE
    vTaskDelay(100);
#endif
	while(i<length){
		for(int j=0; j<16 && i<length; j++) printHex(array[i++]);
		Cy_Debug_AddToLog(1, "\r\n%s", ts);
#if USBFS_LOGS_ENABLE
    vTaskDelay(100);
#endif
	}
	Cy_Debug_AddToLog(1, "> %s length: 0d%d\r\n\r\n", arrayName, length);
#if USBFS_LOGS_ENABLE
    vTaskDelay(100);
#endif
}

/**
 * \name optiga_app_util_encode_ecc_public_key_in_bit_string_format
 * \brief If and when using a pure pubkey that isn't provided by Optiga, this function will add required headers
 * \note This function is unused in this example, as the keypair is generated by the Optiga module includes headers
 * \param q_buffer
 * \param q_length
 * \param pub_key_buffer
 * \param pub_key_length
 * \retval None
 */
void optiga_app_util_encode_ecc_public_key_in_bit_string_format(
    const uint8_t *q_buffer,
    uint8_t q_length,
    uint8_t *pub_key_buffer,
    uint16_t *pub_key_length
) {
#define OPTIGA_EXAMPLE_UTIL_ECC_DER_ADDITIONAL_LENGTH (0x02)

    uint16_t index = 0;

    pub_key_buffer[index++] = OPTIGA_EXAMPLE_UTIL_DER_BITSTRING_TAG;
    pub_key_buffer[index++] = q_length + OPTIGA_EXAMPLE_UTIL_ECC_DER_ADDITIONAL_LENGTH;
    pub_key_buffer[index++] = OPTIGA_EXAMPLE_UTIL_DER_NUM_UNUSED_BITS;
    /* Compression format. Supports only 04 [uncompressed] */
    pub_key_buffer[index++] = 0x04;

    pal_os_memcpy(&pub_key_buffer[index], q_buffer, q_length);
    index += q_length;

    *pub_key_length = index;

#undef OPTIGA_EXAMPLE_UTIL_ECC_DER_ADDITIONAL_LENGTH
}

/**
 * \name optiga_app_performance_measurement
 * \brief Calculate time difference for performance measurements
 * \param time_value
 * \param time_reset_flag
 * \retval None
 */
void optiga_app_performance_measurement(uint32_t *time_value, uint8_t time_reset_flag) {
    if (TRUE == time_reset_flag) {
        *time_value = pal_os_timer_get_time_in_milliseconds();
    } else if (FALSE == time_reset_flag) {
        *time_value = pal_os_timer_get_time_in_milliseconds() - *time_value;
    }
}

/* Metadata of OPTIGA_FREE_ECC_KEY_ID */
const uint8_t OPTIGA_FREE_ECC_KEY_ID_metadata[] = { 0x20, 0x06, 0xD0, 0x01, 0x00, 0xD3, 0x01, 0x00 };

/* SHA-256 Digest to be signed */
static const uint8_t digest[] = {
    /* Size of digest to be chosen based on Curve */
    0x61, 0xC7, 0xDE, 0xF9, 0x0F, 0xD5, 0xCD, 0x7A, 0x8B, 0x7A, 0x36, 0x41, 0x04, 0xE0, 0x0D, 0x82,
    0x38, 0x46, 0xBF, 0xB7, 0x70, 0xEE, 0xBF, 0x8F, 0x40, 0x25, 0x2E, 0x0A, 0x21, 0x42, 0xAF, 0x9C,
};

/* Global variables associated with High BandWidth DMA setup. */
cy_stc_hbdma_context_t HBW_DrvCtxt;     /* High BandWidth DMA driver context. */
cy_stc_hbdma_dscr_list_t HBW_DscrList;  /* High BandWidth DMA descriptor free list. */
cy_stc_hbdma_buf_mgr_t HBW_BufMgr;      /* High BandWidth DMA buffer manager. */
cy_stc_hbdma_mgr_context_t HBW_MgrCtxt; /* High BandWidth DMA manager context. */

/**
 * \name Cy_Optiga_HbDmaInit
 * \brief Initialize HBDMA block.
 * \retval None
 */
bool Cy_Optiga_HbDmaInit(void)
{
    cy_en_hbdma_status_t drvstat;
    cy_en_hbdma_mgr_status_t mgrstat;

    /* Initialize the HBW DMA driver layer. */
    drvstat = Cy_HBDma_Init(LVDSSS_LVDS, USB32DEV, &HBW_DrvCtxt, 0, 0);
    if (drvstat != CY_HBDMA_SUCCESS)
    {
        return false;
    }

    /* Setup a HBW DMA descriptor list. */
    mgrstat = Cy_HBDma_DscrList_Create(&HBW_DscrList, 256U);
    if (mgrstat != CY_HBDMA_MGR_SUCCESS)
    {
        return false;
    }

    /* Initialize the DMA buffer manager. We will use 512 KB of space from 0x1C030000 onwards. */
    mgrstat = Cy_HBDma_BufMgr_Create(&HBW_BufMgr, (uint32_t *)0x1C030000UL, 0x80000UL);
    if (mgrstat != CY_HBDMA_MGR_SUCCESS)
    {
        return false;
    }

    /* Initialize the HBW DMA channel manager. */
    mgrstat = Cy_HBDma_Mgr_Init(&HBW_MgrCtxt, &HBW_DrvCtxt, &HBW_DscrList, &HBW_BufMgr);
    if (mgrstat != CY_HBDMA_MGR_SUCCESS)
    {
        return false;
    }

    return true;
}

/**
 * \name Cy_Optiga_Main
 * \brief The main Optiga application logic
 * \retval None
 */
void Cy_Optiga_Main(void){
	uint32_t time_taken;
	START_PERFORMANCE_MEASUREMENT(time_taken);
    optiga_lib_status_t return_status = !OPTIGA_LIB_SUCCESS;
    optiga_key_id_t optiga_key_id;

    /**
     * It is important to use a separate variable X_length for the public key and
     * the signature to hold lengths as lengths will be modified after keypair
     * generation and signing. The public key result is a populated array of size 
     * 68B (4B header + 64B key).
     */
    uint8_t public_key[100];
    uint16_t public_key_length = sizeof(public_key);
    uint16_t optiga_oid;

    optiga_crypt_t * crypt_me = NULL;
    optiga_util_t * util_me = NULL;

    do
    {
        /**
         * 1. Create OPTIGA Crypt and Util Instances
         */
        crypt_me = optiga_crypt_create(0, optiga_crypt_callback, NULL);
        if (NULL == crypt_me)
        {
            break;
        }

        util_me = optiga_util_create(0, optiga_util_callback, NULL);
        if (NULL == util_me)
        {
            break;
        }

        /* Use a copy for printing, to preserve the const nature of digest */
        uint8_t digest_copy[sizeof(digest)];
        pal_os_memcpy(digest_copy, digest, sizeof(digest));
        printArray16("Digest", digest_copy, sizeof(digest), false); /* Printing the digest */

        /**
         * Write medata for key store OPTIGA_FREE_ECC_KEY_ID
         * This macro is set as part of the lib config header file, to 0xE0F2
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        optiga_oid = OPTIGA_FREE_ECC_KEY_ID;
        return_status = optiga_util_write_metadata(util_me,
                                                   optiga_oid,
                                                   OPTIGA_FREE_ECC_KEY_ID_metadata,
                                                   sizeof(OPTIGA_FREE_ECC_KEY_ID_metadata));
        WAIT_AND_CHECK_STATUS(return_status, optiga_lib_status);
        OPTIGA_LOG_MESSAGE("Metadata Write Complete, Key Store ID: 0x%x", optiga_oid);

        /**
         * 2. Generate ECC Key pair
         *       - Use ECC NIST P 256 Curve
         *       - Specify the Key Usage (Key Agreement or Sign based on requirement)
         *       - Store the Private key in OPTIGA Key store
         *       - Export Public Key
         */
        optiga_lib_status = OPTIGA_LIB_BUSY;
        optiga_key_id = OPTIGA_FREE_ECC_KEY_ID;
        /* For session-based keys, use OPTIGA_KEY_ID_SESSION_BASED as key id as shown below. */
        // optiga_key_id = OPTIGA_KEY_ID_SESSION_BASED;
        return_status = optiga_crypt_ecc_generate_keypair(crypt_me,
                                                          OPTIGA_ECC_CURVE_NIST_P_256,
                                                          (uint8_t)OPTIGA_KEY_USAGE_SIGN,
                                                          FALSE,
                                                          &optiga_key_id,
                                                          public_key,
                                                          &public_key_length);
        WAIT_AND_CHECK_STATUS(return_status, optiga_lib_status);
        OPTIGA_LOG_MESSAGE("Keypair Generation Complete, Key Store ID: 0x%x", optiga_key_id);

        return_status = OPTIGA_LIB_SUCCESS;


        /**
		 * 3. Sign the digest using Private key from Key Store ID E0F2
		 */
        uint8_t signature[80];
        uint16_t signature_length = sizeof(signature);
        optiga_lib_status = OPTIGA_LIB_BUSY;
        return_status = optiga_crypt_ecdsa_sign(
            crypt_me,
            digest,
            sizeof(digest),
            optiga_key_id,
            signature,
            &signature_length
        );
        WAIT_AND_CHECK_STATUS(return_status, optiga_lib_status);
        OPTIGA_LOG_MESSAGE("Signing Complete, Key Store ID: 0x%x", optiga_key_id);

        printArray16("Public Key (incl. header)", public_key, public_key_length, true);
        printArray16("Signature (in DER encoding format)", signature, signature_length, false);


        /**
         * 4. Verify the signature
         */

        /* The following commented code must be enabled if the public_key being used is pure. i.e,
         * if the size is 64Bytes and is not a result generated by Optiga. Make a call to the
         * optiga_app_util_encode_ecc_public_key_in_bit_string_format API, this will add required header
         * tags for a pure public key.
         *
         * Public keys generated by the Optiga module do not require this step as they already contain
         * the required 4B header.
         */
//      uint8_t ecc_public_key[sizeof(public_key)];
//		uint16_t ecc_public_key_length = sizeof(ecc_public_key);
//      optiga_app_util_encode_ecc_public_key_in_bit_string_format(public_key,
//                                                            sizeof(public_key),
//                                                            ecc_public_key,
//                                                            &ecc_public_key_length);
//      pal_os_memcpy(public_key, ecc_public_key, sizeof(ecc_public_key));

        public_key_from_host_t public_key_details = {
                                                     public_key,
                                                     public_key_length,
                                                     (uint8_t)OPTIGA_ECC_CURVE_NIST_P_256
                                                    };
        optiga_lib_status = OPTIGA_LIB_BUSY;
		return_status = optiga_crypt_ecdsa_verify(
			crypt_me,
			digest,
			sizeof(digest),
			signature,
			signature_length,
			OPTIGA_CRYPT_HOST_DATA,
			&public_key_details
		);
		WAIT_AND_CHECK_STATUS(return_status, optiga_lib_status);
		OPTIGA_LOG_MESSAGE("Sign Verification Complete");
#if USBFS_LOGS_ENABLE
    vTaskDelay(100);
#endif
    } while (FALSE);		/* The loop allows us to break on error, and log the error code */
    READ_PERFORMANCE_MEASUREMENT(time_taken);
    OPTIGA_LOG_PERFORMANCE_VALUE(time_taken, return_status);
#if USBFS_LOGS_ENABLE
    vTaskDelay(100);
#endif
    OPTIGA_LOG_STATUS(__FUNCTION__, return_status);
#if USBFS_LOGS_ENABLE
    vTaskDelay(100);
#endif
    if (crypt_me)
    {
        /* Destroy the instance after the completion of usecase if not required. */
        return_status = optiga_crypt_destroy(crypt_me);
        if(OPTIGA_LIB_SUCCESS != return_status)
        {
            //lint --e{774} suppress This is a generic macro
            OPTIGA_LOG_STATUS(__FUNCTION__, return_status);
        }
    }
    if (util_me)
    {
        /* Destroy the instance after the completion of usecase if not required. */
        return_status = optiga_util_destroy(util_me);
        if(OPTIGA_LIB_SUCCESS != return_status)
        {
            //lint --e{774} suppress This is a generic macro
            OPTIGA_LOG_STATUS(__FUNCTION__, return_status);
        }
    }

}
