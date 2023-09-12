#include <iostream>
#include <chrono>
#include <thread>
void pairDevice();
void validateDevice();
void createDCMI();
void pressAnyKeyToEnd();
void deleteDCMI();

int main(){
    //you will need sudo apt install ifuse
    //sudo apt install libimobiledevice-utils
    
    // pairDevice();
    // validateDevice();
    // createDCMI();
    // pressAnyKeyToEnd();
    // deleteDCMI();
}

void pairDevice(){
    using namespace std::chrono_literals;

    int pairSuccess = -1, attempts = 0;
    std::cout << "Pairing to device.." << std::endl;
    while (pairSuccess != 0)
    {
        if(attempts < 5){
            pairSuccess = system("idevicepair pair");
            if(pairSuccess == 0)
                break;

            attempts++;
            std::this_thread::sleep_for(2000ms);
        }else{
            std::cout << "Maximum number of attempts excedded. Exiting." << std::endl;
            exit(1);
        }
    }

    std::cout << "Success." << std::endl <<std::endl;
}

void validateDevice(){
    using namespace std::chrono_literals;

    int validateSuccess = -1, attempts = 0;
    std::cout << "Validating device.." << std::endl;
    while (validateSuccess != 0)
    {
        if(attempts < 5){
            validateSuccess = system("idevicepair validate");
            if(validateSuccess == 0)
                break;

            attempts++;
            std::this_thread::sleep_for(2000ms);
        }else{
            std::cout << "Maximum number of attempts excedded. Exiting." << std::endl;
            exit(1);
        }
    }

    std::cout << "Success." << std::endl <<std::endl;
}

void createDCMI(){
    system("mkdir ~/Desktop/iPhone-Photos");
    system("ifuse ~/Desktop/iPhone-Photos");
}

void deleteDCMI(){
    system("idevicepair unpair");
    system("umount ~/Desktop/iPhone-Photos");
    system("rmdir ~/Desktop/iPhone-Photos");
}

void pressAnyKeyToEnd(){
    std::cout << "Mount created at ~/Desktop/iPhone-Photos" << std::endl;
    std::cout << std::endl << "Press any key to end..";
    std::cin.ignore().get();
}