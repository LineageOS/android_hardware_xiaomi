/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */
///////////////////////////////////////////////////////////////////////////////
// THIS FILE IS IMMUTABLE. DO NOT EDIT IN ANY CASE.                          //
///////////////////////////////////////////////////////////////////////////////

// This file is a snapshot of an AIDL file. Do not edit it manually. There are
// two cases:
// 1). this is a frozen version file - do not edit this in any case.
// 2). this is a 'current' file. If you make a backwards compatible change to
//     the interface (from the latest frozen version), the build system will
//     prompt you to update this file with `m <name>-update-api`.
//
// You must not make a backward incompatible change to any AIDL file built
// with the aidl_interface module type with versions property set. The module
// type is used to build AIDL files in a way that they can be used across
// independently updatable components of the system. If a device is shipped
// with such a backward incompatible change, it has a high risk of breaking
// later when a module using the interface is updated, e.g., Mainline modules.

package vendor.xiaomi.hardware.aidl.mtdservice;
@VintfStability
interface IMTService {
  String eccSign(int i, String str);
  String enroll(String str, int i);
  boolean external_id_load(int i);
  String external_key_dump(int i);
  int external_key_load(int i, String str, String str2);
  String external_key_prepare(int i);
  int external_key_version(int i);
  int fido_key_get_version();
  int fido_key_load(String str, String str2);
  String fido_key_prepare();
  String getFid();
  String getTAVersion();
  int get_device_rmpb_counter();
  int[] get_device_secure_status();
  String ifaa_key_dump();
  int ifaa_key_get_version();
  int ifaa_key_load(String str, String str2);
  String ifaa_key_prepare();
  vendor.xiaomi.hardware.aidl.mtdservice.MTServiceResult persist_read(int i, String str);
  int persist_remove(int i, String str);
  int persist_write(int i, String str, in byte[] bArr, int i2);
  int refreash_device_rpmb_status();
  int reload(String str, String str2);
  String soter_generate();
  int soter_get_state();
  void soter_set_state(int i);
  String widevine_dump();
  int widevine_get_version();
  int widevine_load(String str, String str2);
  String widevine_prepare();
}
