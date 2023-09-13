#include <iostream>
#include <chrono>
#include <thread>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>

#define TOOL_NAME "iphonephotos"
#define MAX_ATTMPTS 8
#define ERR_ATTMPTS "ERROR: Max number of attempts exceeded.\n"

int findAndConnectDevice(idevice_t * device);
int retriveUDID(idevice_t * device, char ** udid);
int pairDevice(lockdownd_client_t * client, idevice_t * device, char * udid);
int validateDevice(lockdownd_client_t * client,  idevice_t * device, char * udid);
void printErrorMessage(lockdownd_error_t err, char * udid);
int mountDevice(char * udid);
std::string getDirectoryName(char * udid);
void pressAnyKeyToEnd();
int unPair(lockdownd_client_t * client, char * udid);
int umountDevice(char * udid);

int main(){
    int result = 0;

    static char *udid = NULL;
    idevice_t device = NULL;
    lockdownd_client_t client = NULL;
    
    result = findAndConnectDevice(&device);
    if(result != 0) goto leave;

    result = retriveUDID(&device, &udid);
    if(result != 0) goto leave;

    result = pairDevice(&client, &device, udid);
    if(result != 0) goto leave;

    result = validateDevice(&client, &device, udid);
    if(result != 0) goto leave;

    result = mountDevice(udid);
    if(result != 0) goto leave;

    pressAnyKeyToEnd();

    result = unPair(&client, udid);
    if(result != 0) goto leave;

    result = umountDevice(udid);
    if(result != 0) goto leave;

leave:
	lockdownd_client_free(client);
	idevice_free(device);
	free(udid);

	return result;
}

int findAndConnectDevice(idevice_t * device){
    using namespace std::chrono_literals;

    int attempts = 0;
    idevice_error_t derr;

    std::cout << "Connecting to device... ";
    std::cout.flush();
    while(true){
        if(attempts < MAX_ATTMPTS){
            derr = idevice_new_with_options(device, NULL, IDEVICE_LOOKUP_USBMUX);
            if(derr == IDEVICE_E_SUCCESS){
                std::cout << "Success!" << std::endl;
                return 0;
            }

            attempts++;
            std::this_thread::sleep_for(2000ms);
        }else{
            std::cout << ERR_ATTMPTS;
            break;
        }
    }

    return 1;
}

int retriveUDID(idevice_t * device, char ** udid){
    using namespace std::chrono_literals;

    int attempts = 0;
    idevice_error_t derr;

    std::cout << "Getting UDID from device... ";
    std::cout.flush();
    while(true){
        if(attempts < MAX_ATTMPTS){
            derr = idevice_get_udid(*device, udid);
            if(derr == IDEVICE_E_SUCCESS){
                std::cout << "Success!" << std::endl;
                return 0;
            }

            attempts++;
            std::this_thread::sleep_for(2000ms);
        }else{
            std::cout << ERR_ATTMPTS;
            break;
        }
    }

    return 1;
}

int pairDevice(lockdownd_client_t * client, idevice_t * device, char * udid){
    using namespace std::chrono_literals;
    
    lockdownd_error_t lerr;
    char *type = NULL;
    int attempts = 0;
    bool error = false;

    std::cout << "Pairing device... ";
    std::cout.flush();
    while (true)
    {
        if(error){
            if(attempts == 0)
                std::cout << std::endl;
            printErrorMessage(lerr, udid);
            attempts++;
            error = false;
            std::this_thread::sleep_for(2000ms);
        }

        if(attempts < MAX_ATTMPTS){
            lerr = lockdownd_client_new(*device, client, TOOL_NAME);
            if(lerr != LOCKDOWN_E_SUCCESS){
                error = true;
                continue;
            }

            lerr = lockdownd_query_type(*client, &type);
            if(lerr != LOCKDOWN_E_SUCCESS){
                error = true;
                continue;
            }

            lerr = lockdownd_pair(*client, NULL);
            if(lerr != LOCKDOWN_E_SUCCESS){
                error = true;
                continue;
            }
            
            free(type);
            std::cout << "Success!" <<  std::endl;
            return 0;
        }
        else{
            std::cout << ERR_ATTMPTS;
            break;
        }
    }
    
    free(type);
    return 1;
}

int validateDevice(lockdownd_client_t * client,  idevice_t * device, char * udid){
    using namespace std::chrono_literals;
    lockdownd_error_t lerr;
    int attempts = 0;

    lockdownd_client_free(*client);
    if(*client != NULL){
        *client = NULL;
    }

    std::cout << "Validating device... ";
    std::cout.flush();
    while(true){
        if(attempts < MAX_ATTMPTS){
            lerr = lockdownd_client_new_with_handshake(*device, client, TOOL_NAME);
            if(lerr == LOCKDOWN_E_SUCCESS){
                std::cout << "Success!" <<std::endl;
                return 0;
            }

            printErrorMessage(lerr, udid);
            attempts++;
            std::this_thread::sleep_for(2000ms);
        }else{
            std::cout << ERR_ATTMPTS;
            break;
        }
    }

    return 1;
}

int mountDevice(char * udid){
    system(("mkdir " + getDirectoryName(udid)).c_str());
    system(("ifuse " + getDirectoryName(udid)).c_str());

    std::cout << "Mount created at " << getDirectoryName(udid) << std::endl;
    return 0;
}

int umountDevice(char * udid){
    system(("umount " + getDirectoryName(udid)).c_str());
    system(("rmdir " + getDirectoryName(udid)).c_str());

    std::cout << "Mount and " << getDirectoryName(udid) << " removed." << std::endl;
    return 0;
}

int unPair(lockdownd_client_t * client, char * udid){
    using namespace std::chrono_literals;

    int attempts = 0;
    lockdownd_error_t lerr;

    std::cout << "Unpairing device... ";
    std::cout.flush();
    do{
        lerr = lockdownd_unpair(*client, NULL);
        if (lerr == LOCKDOWN_E_SUCCESS) {
            std::cout << "Sucess!" << std::endl;
            return 0;
        } else {
            if(attempts == 0) std::cout << std::endl;
            printErrorMessage(lerr, udid);

            attempts++;
            std::this_thread::sleep_for(2000ms);
        }

    }while(lerr != LOCKDOWN_E_SUCCESS || attempts < MAX_ATTMPTS);
    
    if(!(attempts < MAX_ATTMPTS)) std::cout << ERR_ATTMPTS;
    return 1;
}

std::string getDirectoryName(char * udid){
    const char * basePath = "~/Desktop/";
    std::string udidString = std::string(udid);

    return std::string((basePath + udidString).c_str());
}

void pressAnyKeyToEnd(){
    std::cout << std::endl << "Press any key to finish.." << std::endl;
    std::cin.ignore().get();
}

void printErrorMessage(lockdownd_error_t err, char * udid){
	switch (err) {
		case LOCKDOWN_E_PASSWORD_PROTECTED:
            std::cout << "ERROR: Could not validate with device " << udid << " because a passcode is set. Please enter the passcode on the device and retry." << std::endl;
			break;
		case LOCKDOWN_E_INVALID_CONF:
		case LOCKDOWN_E_INVALID_HOST_ID:
            std::cout << "ERROR: Device " << udid << " is not paired with this host" << std::endl;
			break;
		case LOCKDOWN_E_PAIRING_DIALOG_RESPONSE_PENDING:
            std::cout << "ERROR: Please accept the trust dialog on the screen of device " << udid << ", then attempt to pair again." << std::endl;
			break;
		case LOCKDOWN_E_USER_DENIED_PAIRING:
            std::cout << "ERROR: Device " << udid << " said that the user denied the trust dialog." << std::endl;
			break;
		case LOCKDOWN_E_PAIRING_FAILED:
            std::cout << "ERROR: Pairing with device " << udid << " failed." << std::endl;
			break;
		case LOCKDOWN_E_GET_PROHIBITED:
		case LOCKDOWN_E_PAIRING_PROHIBITED_OVER_THIS_CONNECTION:
            std::cout << "ERROR: Pairing is not possible over this connection." << std::endl;
			break;
		default:
            std::cout << "ERROR: Device " << udid << " returned unhandled error code "  << err << std::endl;
			break;
	}
}