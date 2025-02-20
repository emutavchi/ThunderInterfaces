/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include "Module.h"

namespace WPEFramework {

namespace Exchange {

    // This interface gives direct access to a OpenCDMi server instance, running as a plugin in the framework.
    struct EXTERNAL IContentDecryption : virtual public Core::IUnknown {
        enum { ID = ID_CONTENTDECRYPTION };

        virtual uint32_t Initialize(PluginHost::IShell* service) = 0;
        virtual void Deinitialize(PluginHost::IShell* service) = 0;
        virtual uint32_t Reset() = 0;
        virtual RPC::IStringIterator* Systems() const = 0;
        virtual RPC::IStringIterator* Designators(const string& keySystem) const = 0;
        virtual RPC::IStringIterator* Sessions(const string& keySystem) const = 0;
    };



    class DataExchange : public Core::SharedBuffer {
    private:
        DataExchange() = delete;
        DataExchange(const DataExchange&) = delete;
        DataExchange& operator=(const DataExchange&) = delete;

    private:
        struct Administration {
            uint32_t Status;
            uint8_t KeyId[17];
            uint8_t IVLength;
            uint8_t IV[24];
            uint16_t SubLength;
            uint8_t Sub[2048];
            bool InitWithLast15;
        };

    public:
        DataExchange(const string& name)
            : Core::SharedBuffer(name.c_str())
        {
        }
        DataExchange(const string& name, const uint32_t bufferSize)
            : Core::SharedBuffer(name.c_str(), 
                Core::File::USER_READ    |
                Core::File::USER_WRITE   |
                Core::File::USER_EXECUTE |
                Core::File::GROUP_READ   |
                Core::File::GROUP_WRITE  |
                Core::File::OTHERS_READ  |
                Core::File::OTHERS_WRITE,
                bufferSize,
                sizeof(Administration))
        {
            Administration* admin = reinterpret_cast<Administration*>(AdministrationBuffer());
            // Clear the administration space before using it.
            ::memset(admin, 0, sizeof(Administration));
        }
        ~DataExchange() {}

    public:
        inline void Status(uint32_t status)
        {
            reinterpret_cast<Administration*>(AdministrationBuffer())->Status = status;
        }
        inline uint32_t Status() const
        {
            return (reinterpret_cast<const Administration*>(AdministrationBuffer())
                        ->Status);
        }
        inline void InitWithLast15(bool initWithLast15)
        {
            reinterpret_cast<Administration*>(AdministrationBuffer())->InitWithLast15 = initWithLast15;
        }
        inline bool InitWithLast15() const
        {
            return (reinterpret_cast<const Administration*>(AdministrationBuffer())
                        ->InitWithLast15);
        }
        void SetIV(const uint8_t ivDataLength, const uint8_t ivData[])
        {
            Administration* admin = reinterpret_cast<Administration*>(AdministrationBuffer());
            ASSERT(ivDataLength <= sizeof(Administration::IV));
            admin->IVLength = (ivDataLength > sizeof(Administration::IV) ? sizeof(Administration::IV)
                                                                        : ivDataLength);
            ::memcpy(admin->IV, ivData, admin->IVLength);
            if (admin->IVLength < sizeof(Administration::IV)) {
                ::memset(&(admin->IV[admin->IVLength]), 0,
                    (sizeof(Administration::IV) - admin->IVLength));
            }
        }
        void SetSubSampleData(const uint16_t length, const uint8_t* data)
        {
            Administration* admin = reinterpret_cast<Administration*>(AdministrationBuffer());
            admin->SubLength = (length > sizeof(Administration::Sub) ? sizeof(Administration::Sub)
                                                                    : length);
            if (data != nullptr) {
                ::memcpy(admin->Sub, data, admin->SubLength);
            }
        }
        void Write(const uint32_t length, const uint8_t* data)
        {

            if (Core::SharedBuffer::Size(length) == true) {
                SetBuffer(0, length, data);
            }
        }
        void Read(const uint32_t length, uint8_t* data) const
        {
            GetBuffer(0, length, data);
        }
        const uint8_t* IVKey() const
        {
            const Administration* admin = reinterpret_cast<const Administration*>(AdministrationBuffer());
            return (admin->IV);
        }
        uint8_t IVKeyLength() const
        {
            const Administration* admin = reinterpret_cast<const Administration*>(AdministrationBuffer());
            return (admin->IVLength);
        }
        void KeyId(const uint8_t length, const uint8_t buffer[])
        {
            Administration* admin = reinterpret_cast<Administration*>(AdministrationBuffer());
            ASSERT(length <= 16);
            admin->KeyId[0] = (length <= 16 ? length : 16);
            if (length != 0) {
                ::memcpy(&(admin->KeyId[1]), buffer, admin->KeyId[0]);
            }
        }
        const uint8_t* KeyId(uint8_t& length) const
        {
            const Administration* admin = reinterpret_cast<const Administration*>(AdministrationBuffer());
            length = admin->KeyId[0];
            ASSERT(length <= 16);
            return (length > 0 ? &admin->KeyId[1] : nullptr);
        }
    };

}
}

