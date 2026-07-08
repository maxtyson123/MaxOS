//
// Created by 98max on 1/07/2026.
//

#ifndef MAXOS_FSEVENTHANDLER_H
#define MAXOS_FSEVENTHANDLER_H

#include <libdriver/server/drivermanager_client.h>
#include <libdriver/server/drivermanager_types.h>
#include <libdriver/driver.h>


namespace FileServer::filesystem {

	class FSDriverManagerEventHanlder : public EventHandler_drivermanager {

		public:

			FSDriverManagerEventHanlder();
			~FSDriverManagerEventHanlder();

			void on_driver_ready(mstring id) override;
	};


}

#endif //MAXOS_FSEVENTHANDLER_H
