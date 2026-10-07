// SPDX-License-Identifier: Apache-2.0

package vendor.xiaomi.hardware.mlipay;

// AIDL transaction offsets differ from the HIDL method order.
interface IMlipayService {
    int[] ifaa_get_idlist(int bioType) = 0;
    byte[] invoke_command(in byte[] sbuf, int sbuf_len) = 5;
}
