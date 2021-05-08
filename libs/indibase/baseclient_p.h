#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>
#include <set>
#include <thread>
#include <cstdint>

#include <list>
#include <algorithm>

#include <lilxml.h>
#include "basedevice.h"

#ifdef _WINDOWS
#include <WinSock2.h>
#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif
#endif

namespace INDI
{

// #PS: Temporary implementation of BaseDeviceList.
class BaseDeviceList: public std::list<INDI::BaseDevice>
{
public:
    std::list<BaseDevice>::iterator findByName(const char *name)
    {
        return std::find_if(begin(), end(), [name](const INDI::BaseDevice &it)
        {
            return it.getDeviceNameAsString() == name;
        });
    }

    std::list<BaseDevice>::iterator findByName(const std::string &name)
    {
        return findByName(name.c_str());
    }

    BaseDevice &addDevice(const char *name, INDI::BaseMediator *mediator)
    {
        emplace_back();
        auto &it = back();
        it.setDeviceName(name);
        it.setMediator(mediator);
        return it;
    }

    bool removeByName(const char *name)
    {
        auto it = findByName(name);
        if (it == end())
            return false;

        erase(it);

        return true;
    }

    bool removeByName(const std::string &name)
    {
        return removeByName(name.c_str());
    }
};

struct BLOBMode
{
    std::string device;
    std::string property;
    BLOBHandling blobMode;
};

class BaseClientPrivate
{
public:
    BaseClientPrivate(BaseClient *parent);
    virtual ~BaseClientPrivate();

public:
    bool connect();
    bool disconnect(int exit_code);

    /** @brief Connect/Disconnect to INDI driver
     *  @param status If true, the client will attempt to turn on CONNECTION property within the driver (i.e. turn on the device).
     *                Otherwise, CONNECTION will be turned off.
     *  @param deviceName Name of the device to connect to.
     */
    void setDriverConnection(bool status, const char *deviceName);

    size_t sendData(const void *data, size_t size);
    void sendString(const char *fmt, ...);
    
public:
    void listenINDI();
    /** @brief clear Clear devices and blob modes */
    void clear();

public:
    BLOBMode *findBLOBMode(const std::string &device, const std::string &property);

public:
    /** @brief Dispatch command received from INDI server to respective devices handled by the client */
    int dispatchCommand(XMLEle *root, char *errmsg);

    /** @brief Delete property command */
    int delPropertyCommand(XMLEle *root, char *errmsg);

    /** @brief  Process messages */
    int messageCommand(XMLEle *root, char *errmsg);

public:
    /** @brief Remove device from list */
    int removeDevice(const char *devName, char *errmsg);


    /** @brief Add a new device */
    INDI::BaseDevice *addDevice(XMLEle *dep, char *errmsg);
    /** @brief Find a device, and if it doesn't exist, create it if create is set to 1 */
    INDI::BaseDevice *findDevice(XMLEle *root, bool create, char *errmsg);


public:
    BaseClient *parent;

#ifdef _WINDOWS
    SOCKET sockfd;
#else
    int sockfd {-1};
    int receiveFd {-1};
    int sendFd {-1};
#endif

    BaseDeviceList listOfDevices;

    std::set<std::string> cDeviceNames;
    std::list<BLOBMode> blobModes;
    std::map<std::string, std::set<std::string>> cWatchProperties;

    std::string cServer;
    uint32_t cPort;
    std::atomic_bool sConnected;
    std::atomic_bool sAboutToClose;
    std::mutex sSocketBusy;
    std::condition_variable sSocketChanged;
    int sExitCode;
    bool verbose;

    // Parse & FILE buffers for IO

    uint32_t timeout_sec, timeout_us;
};

}
