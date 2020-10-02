#define WIN32_LEAN_AND_MEAN
#include "hid.hpp"
#include <vector>

HidDevice::HidLibSingle::HidLibSingle()
{
	hid_init();
}

HidDevice::HidLibSingle::~HidLibSingle()
{
	hid_exit();
}

HidDevice::HidLibSingle &HidDevice::HidLibSingle::hidlib()
{
	static HidLibSingle single;
	return single;
}

auto HidDevice::Open() const -> IoHandle
{
    //static_assert(std::is_same<::HANDLE, IoHandle::HANDLE>::value, "");
    
    if(*this)
    {
        /*wchar_t lockpath[MAX_PATH];
        wchar_t devid[64];

        if(SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_COMMON_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_DEFAULT, lockpath)))
        {
            if(swprintf(devid, std::size(devid), L"/device_hid_%X_%X.lock", vendor, product) > 0
                && wcscat_s(lockpath, std::size(lockpath), devid) == 0)
            {
                HANDLE hLock = CreateFileW(lockpath, GENERIC_READ | GENERIC_WRITE, 0 /*noshare*//*, nullptr, OPEN_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, nullptr);
                if(hLock != INVALID_HANDLE_VALUE)
                {
                    HANDLE hDev = CreateFileW(this->device.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, 0);
                    return IoHandle(hDev, hLock);
                }
            }
        }*/
        
        hid_device *hDev = hid_open_path(this->device.c_str());
        return IoHandle(hDev, 0/*hLock*/);
    }
    return IoHandle();
}

auto HidDevice::ScanForDevice(uint16_t vendor, uint16_t product) -> HidDevice
{
    HidLibSingle::hidlib();
    HidDevice output;
    hid_device_info *hidenum = hid_enumerate(vendor, product);
    
    if (hidenum != nullptr)
    {
        output = HidDevice(hidenum->path == nullptr? "" : hidenum->path, hidenum->vendor_id, hidenum->product_id);
        
        hid_free_enumeration(hidenum);
    }
    
    return output;
}

HidDevice::IoHandle::~IoHandle()
{
    if(*this)
    {
        hid_close(hDev);
        //CloseHandle(hDev);

        if(hLock != 0 && hLock != nullptr)
        {
            //CloseHandle(hLock);
        }
    }
}

HidDevice::IoHandle::operator bool() const
{
    return (hDev != nullptr);
}

bool HidDevice::IoHandle::GetFeature(void* buffer, size_t size) const
{
    return hid_get_feature_report(hDev, reinterpret_cast<uint8_t*>(buffer), size) != -1;
}

bool HidDevice::IoHandle::SetFeature(const void* buffer, size_t size) const
{
    return hid_send_feature_report(hDev, reinterpret_cast<const uint8_t*>(buffer), size) != -1;
}
