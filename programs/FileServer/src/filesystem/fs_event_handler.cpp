//
// Created by 98max on 1/07/2026.
//

#include <filesystem/fs_event_handler.h>

using namespace LibDriver;
using namespace MaxOS::KPI;
using namespace FileServer;
using namespace FileServer::filesystem;

FSDriverManagerEventHanlder::FSDriverManagerEventHanlder() {
}

FSDriverManagerEventHanlder::~FSDriverManagerEventHanlder() = default;

void FSDriverManagerEventHanlder::on_driver_ready(mstring id) {
	klog("%s event recv\n", id.c_str());
}


