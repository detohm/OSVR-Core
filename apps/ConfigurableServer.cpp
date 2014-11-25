/** @file
    @brief Implementation

    @date 2014

    @author
    Ryan Pavlik
    <ryan@sensics.com>
    <http://sensics.com>

*/

// Copyright 2014 Sensics, Inc.
//
// All rights reserved.
//
// (Final version intended to be licensed under
// the Apache License, Version 2.0)

// Internal Includes
#include <osvr/Server/Server.h>
#include <osvr/Server/RegisterShutdownHandler.h>

// Library/third-party includes
#include <json/value.h>
#include <json/reader.h>

// Standard includes
#include <iostream>
#include <fstream>
#include <exception>

using std::cout;
using std::cerr;
using std::endl;

static osvr::server::ServerPtr server;

/// @brief Shutdown handler function - forcing the server pointer to be global.
void handleShutdown() {
    cout << "Received shutdown signal..." << endl;
    server->signalStop();
}

int main(int argc, char *argv[]) {
    std::string configName = "osvrServerConfig.json";
    if (argc > 2) {
        configName = argv[1];
    }
    cout << "Loading config file '" << configName << "'" << endl;
    Json::Value root;
    Json::Reader reader;
    try {
        std::ifstream config(configName);
        bool parsingSuccessful = reader.parse(config, root);
        if (!parsingSuccessful) {
            cerr << "Error in parsing JSON config file" << endl;
            cerr << reader.getFormattedErrorMessages() << endl;
            return 1;
        }
    } catch (std::exception &e) {
        cerr << "Caught exception loading and parsing JSON config file: "
             << e.what() << endl;
        return 1;
    }

    cout << "Creating server..." << endl;

    server = osvr::server::Server::createLocal();

    cout << "Registering shutdown handler..." << endl;
    osvr::server::registerShutdownHandler<&handleShutdown>();

    cout << "Loading plugins..." << endl;
    const Json::Value plugins = root["plugins"];
    for (Json::ArrayIndex i = 0, e = plugins.size(); i < e; ++i) {
        std::string plugin = plugins[i].asString();
        cout << "Loading plugin '" << plugin << "'..." << endl;
        try {
            server->loadPlugin(plugin);
            cout << "Plugin '" << plugin << "' loaded!\n" << endl;
        } catch (std::exception &e) {
            std::cerr << "Caught exception tring to load " << plugin << ": "
                      << e.what() << std::endl;
            return 1;
        }
    }

    cout << "Triggering a hardware detection..." << endl;
    server->triggerHardwareDetect();

    cout << "Starting server mainloop..." << endl;
    server->startAndAwaitShutdown();

    cout << "Server mainloop exited." << endl;

    return 0;
}