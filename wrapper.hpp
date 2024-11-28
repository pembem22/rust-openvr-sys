#include "openvr/headers/openvr_capi.h"

// Expose symbols hidden in C API for some reason; see
// https://github.com/ValveSoftware/openvr/issues/89
// https://steamcommunity.com/app/358720/discussions/0/405692758722144628/
// https://github.com/ValveSoftware/openvr/commit/6f671fc80947dbccc4a9d27bd1b89d6038d94637
S_API intptr_t VR_InitInternal( EVRInitError *peError, EVRApplicationType eType );
S_API void VR_ShutdownInternal();
S_API bool VR_IsHmdPresent();
S_API intptr_t VR_GetGenericInterface( const char *pchInterfaceVersion, EVRInitError *peError );
S_API bool VR_IsRuntimeInstalled();
S_API const char * VR_GetVRInitErrorAsSymbol( EVRInitError error );
S_API const char * VR_GetVRInitErrorAsEnglishDescription( EVRInitError error );
// Taken from openvr.h
S_API bool VR_IsInterfaceVersionValid( const char *pchInterfaceVersion );

// Taken from openvr.h; use to work around broken ABI for this event in the C header
struct VREvent_Keyboard_t_real
{
	char cNewInput[8];	// Up to 11 bytes of new input
	uint64_t uUserValue;	// Possible flags about the new input
};

// Taken from openvr.h; use to work around broken ABI for this type in the C header
struct VRTextureWithPose_t_real
{
	void* handle; // See ETextureType definition above
	ETextureType eType;
	EColorSpace eColorSpace;
	HmdMatrix34_t mDeviceToAbsoluteTracking; // Actual pose used to render scene textures.
};

/** This interface is provided by vrserver to allow the driver to notify
 * the system when something changes about a device. These changes must
 * not change the serial number or class of the device because those values
 * are permanently associated with the device's index. */
class IVRDriverContext
{
public:
  /** Returns the requested interface. If the interface was not available it will return NULL and fill
   * out the error. */
  virtual void *GetGenericInterface(const char *pchInterfaceVersion, EVRInitError *peError = nullptr) = 0;

  /** Returns the property container handle for this driver */
  virtual DriverHandle_t GetDriverHandle() = 0;
};

const char *IServerTrackedDeviceProvider_Version = "IServerTrackedDeviceProvider_004";

/** This interface must be implemented in each driver. It will be loaded in vrserver.exe */
class IServerTrackedDeviceProvider
{
public:
  /** initializes the driver. This will be called before any other methods are called.
   * If Init returns anything other than VRInitError_None the driver DLL will be unloaded.
   *
   * pDriverHost will never be NULL, and will always be a pointer to a IServerDriverHost interface
   *
   * pchUserDriverConfigDir - The absolute path of the directory where the driver should store user
   *	config files.
   * pchDriverInstallDir - The absolute path of the root directory for the driver.
   */
  virtual EVRInitError Init(IVRDriverContext *pDriverContext) = 0;

  /** cleans up the driver right before it is unloaded */
  virtual void Cleanup() = 0;

  /** Returns the version of the ITrackedDeviceServerDriver interface used by this driver */
  virtual const char *const *GetInterfaceVersions() = 0;

  /** Allows the driver do to some work in the main loop of the server. */
  virtual void RunFrame() = 0;

  // ------------  Power State Functions ----------------------- //

  /** Returns true if the driver wants to block Standby mode. */
  virtual bool ShouldBlockStandbyMode() = 0;

  /** Called when the system is entering Standby mode. The driver should switch itself into whatever sort of low-power
   * state it has. */
  virtual void EnterStandby() = 0;

  /** Called when the system is leaving Standby mode. The driver should switch itself back to
  full operation. */
  virtual void LeaveStandby() = 0;
};