/** \mainpage
  *  Mr.LOOP WiGig Software Development Kits
  *
  * This library is provided to utilize our devices with limited warranty.
  *
  * Permission is granted to our clients to use this library for any purpose,
  * including commercial applications freely,
  * subject to the following restrictions:
  *
  * 1. The origin of this software must not be misrepresented; you must not
  *    claim that you wrote the original software. If you use this software
  *    in a product, an acknowledgment in the product documentation would be
  *    appreciated but is not required.
  * 2. This notice may not be removed or altered from any source distribution.
  *
  * Copyright (C) 2015-2017 Mr.LOOP Cor. Ltd <software@mrloop.com.tw>
  */

#ifndef MRLOOPBF_RELEASE_H
#define MRLOOPBF_RELEASE_H

#include <stdint.h>

#if defined(MRLOOPBF_LIBRARY)
#  define MRLOOPBFSHARED_EXPORT Q_DECL_EXPORT
#else
#ifdef C_LINUX
#  define MRLOOPBFSHARED_EXPORT
#else
#  define MRLOOPBFSHARED_EXPORT
#endif
#endif

/** \ingroup asyncio
 * Initial the device and SDK first.
 * \returns If return 0, is initialization finish. The other return number is device driver, no device is attached or connect fail.
 */

MRLOOPBFSHARED_EXPORT int ML_Init();

/** \ingroup asyncio
 * Close the device and leave SDK .
 * \returns no return.
 */
MRLOOPBFSHARED_EXPORT void ML_Close();

/** \ingroup asyncio
 * Hide the debug output message.
 * \returns no return.
 */
MRLOOPBFSHARED_EXPORT void ML_HiddenDebugMsg();

/** \ingroup asyncio
 * To send out packet. Timeout value is 1 second.
 * \param In_byte_ptr The buffer to be sent out. The buffer size must be multiple of 4096bytes since the unit in RF transaction is 4096bytes. The buffer will be divided into 4k packets in transaction. Each packet may be dropped or repeat in RF transaction.
 * \param In_length_ptr The size of the buffer.
 * \returns If return fail, there may be error on device driver or no device is attached.
 *
 * ML_Transfer does NOT guarantee that the packet is delivered without error. Even ML_Transfer returns true, the packet could be dropped or repeated.
 * The safe way is to put an index in the packet. Tx sends out the packet. Rx receives the packet and checks the index. Then Rx sends out one packet to note ack.
 * If Tx does not receive the ack packet, then sends out the packet again or return error.
 */
MRLOOPBFSHARED_EXPORT int ML_Transfer(uint8_t* In_byte_ptr, int In_length_ptr);

/** \ingroup asyncio
 * To listen to RF. Timeout value is 1 second.
 * \param In_byte_ptr A buffer to receive data. The buffer size must be multiple of 4096bytes since the unit in RF transaction is 4096bytes. The buffer will be divided into 4k packets in transaction. Each packet may be dropped or repeat in RF transaction.
 * \param In_length_ptr Input the size of the buffer and output the size  of the receive packet. When it returns fail, *In_length_ptr will be 0.
 * \returns When it returns fail, *In_length_ptr will be 0.
 *
 * ML_Transfer does NOT guarantee that the packet is delivered without error. Even ML_Transfer returns true, the packet could be dropped or repeated.
 * The safe way is to put an index in the packet. Tx sends out the packet. Rx receives the packet and checks the index. Then Rx sends out one packet to note ack.
 * If Tx does not receive the ack packet, then sends out the packet again or return error.
 */
MRLOOPBFSHARED_EXPORT int ML_Receiver(uint8_t* In_byte_ptr, int* In_length);

/** \ingroup asyncio
 * is set Mrloop WiGig Dongle speed.
 * \param speed It ranges between 1~7.
 * \returns False is fail.
 */
MRLOOPBFSHARED_EXPORT bool ML_SetSpeed(uint8_t speed);

/** \ingroup asyncio
 * is Set Mrloop WiGig Dongle RF rule
 * \param mode Set mode value "1" is Master, mode value "2" is Slave.
 * \returns False is fail.
 *  Slave can Only connect to Master. Master can Only connect to Slave.
 */
MRLOOPBFSHARED_EXPORT bool ML_SetMode(uint8_t mode);

/** \ingroup asyncio
 *  Get Current USB Device Generation.
 * \returns Integer value when succeeded, 0 if it fails.
 *  1 is USB 1.1
 *  2 is USB 2.0
 *  3 is USB 2.1
 *  4 is USB 3.0
 *  5 is USB 3.1
 */
MRLOOPBFSHARED_EXPORT uint8_t ML_GetDevGen(void);

#endif
