#ifndef __LYKOV_ADD_IN_NATIVE_HPP__
#define __LYKOV_ADD_IN_NATIVE_HPP__

#include <from1C/ComponentBase.h>

#if ENABLE_BASE_FUNCTION
	#include <extensions/extensionBaseFunction.hpp>
#elif ENABLE_HTTP_CLIENT
	#include <extensions/HTTPClient.hpp>
#elif ENABLE_POSTGRE_SQL
	#include <extensions/postgreSQL.hpp>
#elif ENABLE_MY_SQL
	#include <extensions/mySQLConn.hpp>
#endif

AppCapabilities gl_Capabilities = eAppCapabilitiesInvalid;

#endif
