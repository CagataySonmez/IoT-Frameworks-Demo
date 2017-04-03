//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// OCClient.cpp : Defines the entry point for the console application.
//
#include "iotivity_config.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <Windows.h>
#endif
#include <string>
#include <map>
#include <cstdlib>
#include <mutex>
#include <condition_variable>
#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;

static const char* SVR_DB_FILE_NAME = "./oic_svr_db_client.dat";
typedef std::map<OCResourceIdentifier, std::shared_ptr<OCResource>> DiscoveredResourceMap;

DiscoveredResourceMap discoveredResources;
std::shared_ptr<OCResource> curResource;
static OCConnectivityType TRANSPORT_TYPE_TO_USE = OCConnectivityType::CT_ADAPTER_IP;
std::mutex curResourceLock;
void showInteractiveMenu();

class GenericDevice
{
public:

    bool m_state;
    
    GenericDevice() : m_state(false)
    {
    }
};

GenericDevice mydevice;

int observe_count()
{
    static int oc = 0;
    return ++oc;
}
void onObserve(const HeaderOptions /*headerOptions*/, const OCRepresentation& rep,
                    const int& eCode, const int& sequenceNumber)
{
    try
    {
        if(eCode == OC_STACK_OK && sequenceNumber <= MAX_SEQUENCE_NUMBER)
        {
            if(sequenceNumber == OC_OBSERVE_REGISTER)
            {
                std::cout << "Observe registration action is successful" << std::endl;
            }

            std::cout << "OBSERVE RESULT:"<<std::endl;
            std::cout << "\tSequenceNumber: "<< sequenceNumber << std::endl;
            rep.getValue("state", mydevice.m_state);

            std::cout << "\tstate: " << mydevice.m_state << std::endl;

            if(observe_count() == 11)
            {
                std::cout<<"Cancelling Observe..."<<std::endl;
                OCStackResult result = curResource->cancelObserve();

                std::cout << "Cancel result: "<< result <<std::endl;
                sleep(10);
                std::cout << "DONE"<<std::endl;
                std::exit(0);
            }
        }
        else
        {
            if(eCode == OC_STACK_OK)
            {
                std::cout << "No observe option header is returned in the response." << std::endl;
                std::cout << "For a registration request, it means the registration failed"
                        << std::endl;
            }
            else
            {
                std::cout << "onObserve Response error: " << eCode << std::endl;
                std::exit(-1);
            }
        }
    }
    catch(std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onObserve" << std::endl;
    }

}

void onPost(const HeaderOptions& /*headerOptions*/,
        const OCRepresentation& rep, const int eCode)
{
    try
    {
        if(eCode == OC_STACK_OK || eCode == OC_STACK_RESOURCE_CREATED
                || eCode == OC_STACK_RESOURCE_CHANGED)
        {
            std::cout << "**  POST request was successful **" << std::endl;
            std::cout << "press enter to go main menu..." << std::endl;
            system("read");
            showInteractiveMenu();
        }
        else
        {
            std::cout << "onPost Response error: " << eCode << std::endl;
            std::exit(-1);
        }
    }
    catch(std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onPost" << std::endl;
    }
}

// Local function to put a different state for this resource
void postGenericDeviceRepresentation(std::shared_ptr<OCResource> resource)
{
    if(resource)
    {
        OCRepresentation rep;

        std::cout << "Posting device representation..."<<std::endl;

        mydevice.m_state = false;

        rep.setValue("state", mydevice.m_state);

        // Invoke resource's post API with rep, query map and the callback parameter
        resource->post(rep, QueryParamsMap(), &onPost);
    }
}

// Callback handler on GET request
void onGet(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    try
    {
        if(eCode == OC_STACK_OK)
        {
            std::cout << "** GET request was successful **" << std::endl;
            std::cout << "Resource URI: " << rep.getUri() << std::endl;

            rep.getValue("state", mydevice.m_state);

            std::cout << "Device state is: " << (mydevice.m_state == true ? "\033[1;32mON\033[0m" : "\033[1;31mOFF\033[0m") << std::endl;
  
            std::cout << "press enter to go main menu..." << std::endl;
            system("read");
            showInteractiveMenu();
        }
        else
        {
            std::cout << "onGET Response error: " << eCode << std::endl;
            std::exit(-1);
        }
    }
    catch(std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onGet" << std::endl;
    }
}

// Callback to found resources
void foundResource(std::shared_ptr<OCResource> resource)
{
    std::string resourceURI;
    std::string hostAddress;
    try
    {
        {
            std::lock_guard<std::mutex> lock(curResourceLock);
            if(discoveredResources.find(resource->uniqueIdentifier()) == discoveredResources.end())
            {
                std::cout << "Found resource " << resource->uniqueIdentifier() <<
                    " for the first time on server with ID: "<< resource->sid()<<std::endl;
                discoveredResources[resource->uniqueIdentifier()] = resource;
            }
            else
            {
                std::cout<<"Found resource "<< resource->uniqueIdentifier() << " again!"<<std::endl;
            }

            if(curResource)
            {
                std::cout << "Found another resource, ignoring"<<std::endl;
                return;
            }
        }

        // Do some operations with resource object.
        if(resource)
        {
            std::cout<<"DISCOVERED Resource:"<<std::endl;
            // Get the resource URI
            resourceURI = resource->uri();
            std::cout << "\tURI of the resource: " << resourceURI << std::endl;

            // Get the resource host address
            hostAddress = resource->host();
            std::cout << "\tHost address of the resource: " << hostAddress << std::endl;

            // Get the resource types
            std::cout << "\tList of resource types: " << std::endl;
            for(auto &resourceTypes : resource->getResourceTypes())
            {
                std::cout << "\t\t" << resourceTypes << std::endl;
            }

            // Get the resource interfaces
            std::cout << "\tList of resource interfaces: " << std::endl;
            for(auto &resourceInterfaces : resource->getResourceInterfaces())
            {
                std::cout << "\t\t" << resourceInterfaces << std::endl;
            }

            // Get Resource current host
            std::cout << "\tHost of resource: " << std::endl;
            std::cout << "\t\t" << resource->host() << std::endl;

            // Get Resource Endpoint Infomation
            std::cout << "\tList of resource endpoints: " << std::endl;
            for(auto &resourceEndpoints : resource->getAllHosts())
            {
                std::cout << "\t\t" << resourceEndpoints << std::endl;
            }

            // If resource is found from ip based adapter.
            if (std::string::npos != resource->host().find("coap://") ||
                std::string::npos != resource->host().find("coaps://") ||
                std::string::npos != resource->host().find("coap+tcp://") ||
                std::string::npos != resource->host().find("coaps+tcp://"))
            {
                for(auto &resourceEndpoints : resource->getAllHosts())
                {
                    if (resourceEndpoints.compare(resource->host()) != 0 &&
                        std::string::npos == resourceEndpoints.find("coap+rfcomm"))
                    {
                        std::string newHost = resourceEndpoints;

                        if (std::string::npos != newHost.find("tcp"))
                        {
                            TRANSPORT_TYPE_TO_USE = OCConnectivityType::CT_ADAPTER_TCP;
                        }
                        else
                        {
                            TRANSPORT_TYPE_TO_USE = OCConnectivityType::CT_ADAPTER_IP;
                        }
                        // Change Resource host if another host exists
                        std::cout << "\tChange host of resource endpoints" << std::endl;
                        std::cout << "\t\t" << "Current host is "
                                  << resource->setHost(newHost) << std::endl;
                        break;
                    }
                }
            }

            if(resourceURI == "/a/device")
            {
                if (resource->connectivityType() & TRANSPORT_TYPE_TO_USE)
                {
                    curResource = resource;
                    // Get the resource host address
                    std::cout << "\tAddress of selected resource: " << resource->host() << std::endl;

                    sleep(1);
                    showInteractiveMenu();
                }
            }
        }
        else
        {
            // Resource is invalid
            std::cout << "Resource is invalid" << std::endl;
        }

    }
    catch(std::exception& e)
    {
        std::cerr << "Exception in foundResource: "<< e.what() << std::endl;
    }
}

/*
* CagatayS|02.04.2017
* Interactive menu for some specific operations.
* In this demo, we just use coap put and get operations
*/
void showInteractiveMenu()
{
    char input;
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Select an operation" << std::endl;
    std::cout << "1- Get device status" << std::endl;
    std::cout << "2- Turn on device" << std::endl;
    std::cout << "3- Turn off device" << std::endl;
    std::cout << "q- Exit" << std::endl;

    std::cin >> input;

    switch (input)
    {
        case '1':
        {
            std::cout << "getting device representation..."<<std::endl;
            QueryParamsMap test;
            curResource->get(test, &onGet);
            break;
        }
        case '2':
        {
            std::cout << "Posting device representation..."<<std::endl;
            OCRepresentation rep;
            mydevice.m_state = true;
            rep.setValue("state", mydevice.m_state);
            // Invoke resource's post API with rep, query map and the callback parameter
            curResource->post(rep, QueryParamsMap(), &onPost);
            break;
        }
        case '3':
        {
            std::cout << "Posting device representation..."<<std::endl;
            OCRepresentation rep;
            mydevice.m_state = false;
            rep.setValue("state", mydevice.m_state);
            // Invoke resource's post API with rep, query map and the callback parameter
            curResource->post(rep, QueryParamsMap(), &onPost);
            break;
        }
        case 'q':
        {
            std::cout << "good bye..." << std::endl;
            std::exit(0);
        }
        default:
        {
            std::cout << "Invalid entry. Please try again..." << std::endl;
            break;
        }
    }
}

static FILE* client_open(const char* path, const char* mode)
{
    if (0 == strcmp(path, OC_SECURITY_DB_DAT_FILE_NAME))
    {
        return fopen(SVR_DB_FILE_NAME, mode);
    }
    else
    {
        return fopen(path, mode);
    }
}

int main(int argc, char* argv[]) {

    std::ostringstream requestURI;
    OCPersistentStorage ps {client_open, fread, fwrite, fclose, unlink };

    // Create PlatformConfig object
    PlatformConfig cfg {
        OC::ServiceType::InProc,
        OC::ModeType::Both,
        &ps
    };

    cfg.transportType = static_cast<OCTransportAdapter>(OCTransportAdapter::OC_ADAPTER_IP | 
                                                        OCTransportAdapter::OC_ADAPTER_TCP);
    cfg.QoS = OC::QualityOfService::HighQos;

    OCPlatform::Configure(cfg);
    try
    {
        OC_VERIFY(OCPlatform::start() == OC_STACK_OK);

        // makes it so that all boolean values are printed as 'true/false' in this stream
        std::cout.setf(std::ios::boolalpha);

		/*
		* CagatayS|02.04.2017
		* Let user to discover all devices or a specific device
		*/
        char input;
        std::cout << "-----------------------------------" << std::endl;
        std::cout << "Select an operation" << std::endl;
        std::cout << "1- Find all devices" << std::endl;
        std::cout << "2- Find demo device" << std::endl;

        std::cin >> input;

        switch (input)
        {
            case '1':
            {
                // Find all devices
                requestURI << OC_RSRVD_WELL_KNOWN_URI;
                break;
            }
            default:
            {
                // Find demo device
                requestURI << OC_RSRVD_WELL_KNOWN_URI << "?rt=core.device";
                break;
            }
        }

        OCPlatform::findResource("", requestURI.str(),
                CT_DEFAULT, &foundResource);
        std::cout<< "Finding Resource... " <<std::endl;

        // Find resource is done twice so that we discover the original resources a second time.
        // These resources will have the same uniqueidentifier (yet be different objects), so that
        // we can verify/show the duplicate-checking code in foundResource(above);
        OCPlatform::findResource("", requestURI.str(),
                CT_DEFAULT, &foundResource);
        std::cout<< "Finding Resource for second time..." << std::endl;

        // A condition variable will free the mutex it is given, then do a non-
        // intensive block until 'notify' is called on it.  In this case, since we
        // don't ever call cv.notify, this should be a non-processor intensive version
        // of while(true);
        std::mutex blocker;
        std::condition_variable cv;
        std::unique_lock<std::mutex> lock(blocker);
        cv.wait(lock);

        // Perform platform clean up.
        OC_VERIFY(OCPlatform::stop() == OC_STACK_OK);

    }catch(OCException& e)
    {
        oclog() << "Exception in main: "<<e.what();
    }

    return 0;
}


