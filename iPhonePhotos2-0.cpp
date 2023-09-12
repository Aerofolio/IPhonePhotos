#include <iostream>
#include <chrono>
#include <thread>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>

#define TOOL_NAME "iphonephotos"
#define MAX_ATTMPTS 5

int findAndConnectDevice(idevice_t * device);
int retriveUDID(idevice_t * device, char ** udid);

int main(){
    int result = 0;
    static char *udid = NULL;
    lockdownd_error_t lerr;

    idevice_t device = NULL;
    lockdownd_client_t client;

    char *type = NULL;
    
    result = findAndConnectDevice(&device);
    if(result != 0) goto leave;

    result = retriveUDID(&device, &udid);
    if(result != 0) goto leave;
    

    // /* Outputs device identifier */
    // printf("Connected with UDID: %s\n", udid);
    // lerr = lockdownd_client_new(device, &client, TOOL_NAME);
    // lerr = lockdownd_query_type(client, &type);
    // lerr = lockdownd_pair(client, NULL);
    // client = NULL;
    // lerr = lockdownd_client_new_with_handshake(device, &client, TOOL_NAME);
    // /* Cleanup */
    // idevice_free(device);
    // free(udid);

leave:
	lockdownd_client_free(client);
	idevice_free(device);
	free(udid);
    free(type);

	return result;
}

int findAndConnectDevice(idevice_t * device){
    using namespace std::chrono_literals;

    int attempts = 0;
    idevice_error_t derr;

    std::cout << "Connecting to device.. ";
    while(true){
        if(attempts < MAX_ATTMPTS){
            derr = idevice_new_with_options(device, NULL, IDEVICE_LOOKUP_USBMUX);
            if(derr == IDEVICE_E_SUCCESS){
                std::cout << "Success!" <<std::endl;
                return 0;
            }

            attempts++;
            std::this_thread::sleep_for(2000ms);
        }else{
            std::cout << "Error!" << std::endl;
        }
    }

    return 1;
}

int retriveUDID(idevice_t * device, char ** udid){
    using namespace std::chrono_literals;

    int attempts = 0;
    idevice_error_t derr;

    std::cout << "Getting UDID from device.. ";
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
            std::cout << "Error!" << std::endl;
        }
    }

    return 1;
}