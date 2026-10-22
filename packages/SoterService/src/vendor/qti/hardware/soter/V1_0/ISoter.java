/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.qti.hardware.soter.V1_0;

import android.os.HidlSupport;
import android.os.HwBinder;
import android.os.HwParcel;
import android.os.IHwBinder;
import android.os.IHwInterface;
import android.os.RemoteException;

import java.util.ArrayList;
import java.util.Objects;

public interface ISoter extends IHwInterface {
    String kInterfaceName = "vendor.qti.hardware.soter@1.0::ISoter";

    // Transaction IDs aligned with Qualcomm HAL
    int TRANSACTION_getDeviceId = 4;
    int TRANSACTION_generateAskKeyPair = 5;
    int TRANSACTION_exportAskPublicKey = 6;
    int TRANSACTION_removeAllUidKey = 7;
    int TRANSACTION_hasAskAlready = 8;
    int TRANSACTION_generateAuthKeyPair = 9;
    int TRANSACTION_exportAuthKeyPublicKey = 10;
    int TRANSACTION_removeAuthKey = 11;
    int TRANSACTION_hasAuthKey = 12;
    int TRANSACTION_initSign = 13;
    int TRANSACTION_finishSign = 14;
    int TRANSACTION_interfaceChain = 256067662;
    int TRANSACTION_interfaceDescriptor = 256136003;

    static ISoter asInterface(IHwBinder binder) {
        if (binder == null) {
            return null;
        }

        IHwInterface iface = binder.queryLocalInterface(kInterfaceName);
        if (iface instanceof ISoter) {
            return (ISoter) iface;
        }

        ISoter proxy = new ISoter.Proxy(binder);
        try {
            for (String descriptor : proxy.interfaceChain()) {
                if (descriptor.equals(kInterfaceName)) {
                    return proxy;
                }
            }
        } catch (RemoteException e) {
        }

        return null;
    }

    static ISoter getService(String serviceName) throws RemoteException {
        return ISoter.asInterface(HwBinder.getService(kInterfaceName, serviceName));
    }

    static ISoter getService() throws RemoteException {
        return ISoter.getService("default");
    }

    void exportAskPublicKey(int uid, exportAskPublicKeyCallback _hidl_cb) throws RemoteException;

    void exportAuthKeyPublicKey(int uid, String kname, exportAuthKeyPublicKeyCallback _hidl_cb)
            throws RemoteException;

    void finishSign(long signSession, finishSignCallback _hidl_cb) throws RemoteException;

    int generateAskKeyPair(int uid) throws RemoteException;

    int generateAuthKeyPair(int uid, String kname) throws RemoteException;

    void getDeviceId(getDeviceIdCallback _hidl_cb) throws RemoteException;

    int hasAskAlready(int uid) throws RemoteException;

    int hasAuthKey(int uid, String kname) throws RemoteException;

    void initSign(int uid, String kname, String challenge, initSignCallback _hidl_cb)
            throws RemoteException;

    ArrayList<String> interfaceChain() throws RemoteException;

    int removeAllUidKey(int uid) throws RemoteException;

    int removeAuthKey(int uid, String kname) throws RemoteException;

    @FunctionalInterface
    interface exportAskPublicKeyCallback {
        void onValues(int error, ArrayList<Byte> askPublicKey, int askPublicKeyLength);
    }

    @FunctionalInterface
    interface exportAuthKeyPublicKeyCallback {
        void onValues(int error, ArrayList<Byte> authKeyPublicKey, int authKeyPublicKeyLength);
    }

    @FunctionalInterface
    interface finishSignCallback {
        void onValues(int error, ArrayList<Byte> result, int resultLength);
    }

    @FunctionalInterface
    interface getDeviceIdCallback {
        void onValues(int error, ArrayList<Byte> deviceId, int deviceIdLength);
    }

    @FunctionalInterface
    interface initSignCallback {
        void onValues(int error, long session);
    }

    final class Proxy implements ISoter {
        private final IHwBinder mRemote;

        public Proxy(IHwBinder binder) {
            mRemote = Objects.requireNonNull(binder);
        }

        @Override
        public IHwBinder asBinder() {
            return mRemote;
        }

        @Override
        public final boolean equals(Object other) {
            return HidlSupport.interfacesEqual(this, other);
        }

        @Override
        public final int hashCode() {
            return asBinder().hashCode();
        }

        @Override
        public String toString() {
            try {
                return interfaceDescriptor() + "@Proxy";
            } catch (RemoteException e) {
                return "[class or subclass of vendor.qti.hardware.soter@1.0::ISoter]@Proxy";
            }
        }

        public String interfaceDescriptor() throws RemoteException {
            HwParcel _hidl_request = new HwParcel();
            _hidl_request.writeInterfaceToken("android.hidl.base@1.0::IBase");

            HwParcel _hidl_reply = new HwParcel();
            try {
                mRemote.transact(TRANSACTION_interfaceDescriptor, _hidl_request, _hidl_reply, 0);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                return _hidl_reply.readString();
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public ArrayList<String> interfaceChain() throws RemoteException {
            HwParcel _hidl_request = new HwParcel();
            _hidl_request.writeInterfaceToken("android.hidl.base@1.0::IBase");

            HwParcel _hidl_reply = new HwParcel();
            try {
                mRemote.transact(TRANSACTION_interfaceChain, _hidl_request, _hidl_reply, 0);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                return _hidl_reply.readStringVector();
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void getDeviceId(getDeviceIdCallback _hidl_cb) throws RemoteException {
            HwParcel _hidl_request = new HwParcel();
            _hidl_request.writeInterfaceToken(kInterfaceName);

            HwParcel _hidl_reply = new HwParcel();
            try {
                mRemote.transact(TRANSACTION_getDeviceId, _hidl_request, _hidl_reply, 0);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_error = _hidl_reply.readInt32();
                ArrayList<Byte> _hidl_out_deviceId = _hidl_reply.readInt8Vector();
                int _hidl_out_deviceIdLength = _hidl_reply.readInt32();
                _hidl_cb.onValues(_hidl_out_error, _hidl_out_deviceId, _hidl_out_deviceIdLength);
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public int generateAskKeyPair(int uid) throws RemoteException {
            HwParcel _hidl_request = new HwParcel();
            _hidl_request.writeInterfaceToken(kInterfaceName);
            _hidl_request.writeInt32(uid);

            HwParcel _hidl_reply = new HwParcel();
            try {
                mRemote.transact(TRANSACTION_generateAskKeyPair, _hidl_request, _hidl_reply, 0);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                return _hidl_reply.readInt32();
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void exportAskPublicKey(int uid, exportAskPublicKeyCallback _hidl_cb)
                throws RemoteException {
            HwParcel _hidl_request = new HwParcel();
            _hidl_request.writeInterfaceToken(kInterfaceName);
            _hidl_request.writeInt32(uid);

            HwParcel _hidl_reply = new HwParcel();
            try {
                mRemote.transact(TRANSACTION_exportAskPublicKey, _hidl_request, _hidl_reply, 0);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_error = _hidl_reply.readInt32();
                ArrayList<Byte> _hidl_out_askPublicKey = _hidl_reply.readInt8Vector();
                int _hidl_out_askPublicKeyLength = _hidl_reply.readInt32();
                _hidl_cb.onValues(
                        _hidl_out_error, _hidl_out_askPublicKey, _hidl_out_askPublicKeyLength);
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public int removeAllUidKey(int uid) throws RemoteException {
            HwParcel _hidl_request = new HwParcel();
            _hidl_request.writeInterfaceToken(kInterfaceName);
            _hidl_request.writeInt32(uid);

            HwParcel _hidl_reply = new HwParcel();
            try {
                mRemote.transact(TRANSACTION_removeAllUidKey, _hidl_request, _hidl_reply, 0);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                return _hidl_reply.readInt32();
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public int hasAskAlready(int uid) throws RemoteException {
            HwParcel _hidl_request = new HwParcel();
            _hidl_request.writeInterfaceToken(kInterfaceName);
            _hidl_request.writeInt32(uid);

            HwParcel _hidl_reply = new HwParcel();
            try {
                mRemote.transact(TRANSACTION_hasAskAlready, _hidl_request, _hidl_reply, 0);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                return _hidl_reply.readInt32();
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public int generateAuthKeyPair(int uid, String kname) throws RemoteException {
            HwParcel _hidl_request = new HwParcel();
            _hidl_request.writeInterfaceToken(kInterfaceName);
            _hidl_request.writeInt32(uid);
            _hidl_request.writeString(kname);

            HwParcel _hidl_reply = new HwParcel();
            try {
                mRemote.transact(TRANSACTION_generateAuthKeyPair, _hidl_request, _hidl_reply, 0);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                return _hidl_reply.readInt32();
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void exportAuthKeyPublicKey(
                int uid, String kname, exportAuthKeyPublicKeyCallback _hidl_cb)
                throws RemoteException {
            HwParcel _hidl_request = new HwParcel();
            _hidl_request.writeInterfaceToken(kInterfaceName);
            _hidl_request.writeInt32(uid);
            _hidl_request.writeString(kname);

            HwParcel _hidl_reply = new HwParcel();
            try {
                mRemote.transact(TRANSACTION_exportAuthKeyPublicKey, _hidl_request, _hidl_reply, 0);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_error = _hidl_reply.readInt32();
                ArrayList<Byte> _hidl_out_authKeyPublicKey = _hidl_reply.readInt8Vector();
                int _hidl_out_authKeyPublicKeyLength = _hidl_reply.readInt32();
                _hidl_cb.onValues(
                        _hidl_out_error,
                        _hidl_out_authKeyPublicKey,
                        _hidl_out_authKeyPublicKeyLength);
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public int removeAuthKey(int uid, String kname) throws RemoteException {
            HwParcel _hidl_request = new HwParcel();
            _hidl_request.writeInterfaceToken(kInterfaceName);
            _hidl_request.writeInt32(uid);
            _hidl_request.writeString(kname);

            HwParcel _hidl_reply = new HwParcel();
            try {
                mRemote.transact(TRANSACTION_removeAuthKey, _hidl_request, _hidl_reply, 0);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                return _hidl_reply.readInt32();
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public int hasAuthKey(int uid, String kname) throws RemoteException {
            HwParcel _hidl_request = new HwParcel();
            _hidl_request.writeInterfaceToken(kInterfaceName);
            _hidl_request.writeInt32(uid);
            _hidl_request.writeString(kname);

            HwParcel _hidl_reply = new HwParcel();
            try {
                mRemote.transact(TRANSACTION_hasAuthKey, _hidl_request, _hidl_reply, 0);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                return _hidl_reply.readInt32();
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void initSign(int uid, String kname, String challenge, initSignCallback _hidl_cb)
                throws RemoteException {
            HwParcel _hidl_request = new HwParcel();
            _hidl_request.writeInterfaceToken(kInterfaceName);
            _hidl_request.writeInt32(uid);
            _hidl_request.writeString(kname);
            _hidl_request.writeString(challenge);

            HwParcel _hidl_reply = new HwParcel();
            try {
                mRemote.transact(TRANSACTION_initSign, _hidl_request, _hidl_reply, 0);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_error = _hidl_reply.readInt32();
                long _hidl_out_session = _hidl_reply.readInt64();
                _hidl_cb.onValues(_hidl_out_error, _hidl_out_session);
            } finally {
                _hidl_reply.release();
            }
        }

        @Override
        public void finishSign(long signSession, finishSignCallback _hidl_cb)
                throws RemoteException {
            HwParcel _hidl_request = new HwParcel();
            _hidl_request.writeInterfaceToken(kInterfaceName);
            _hidl_request.writeInt64(signSession);

            HwParcel _hidl_reply = new HwParcel();
            try {
                mRemote.transact(TRANSACTION_finishSign, _hidl_request, _hidl_reply, 0);
                _hidl_reply.verifySuccess();
                _hidl_request.releaseTemporaryStorage();

                int _hidl_out_error = _hidl_reply.readInt32();
                ArrayList<Byte> _hidl_out_result = _hidl_reply.readInt8Vector();
                int _hidl_out_resultLength = _hidl_reply.readInt32();
                _hidl_cb.onValues(_hidl_out_error, _hidl_out_result, _hidl_out_resultLength);
            } finally {
                _hidl_reply.release();
            }
        }
    }
}
