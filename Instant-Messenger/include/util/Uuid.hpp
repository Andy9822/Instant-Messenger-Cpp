#ifndef UUID_HPP
#define UUID_HPP
#include <string>

using namespace std;

class Uuid
{
	public:
		Uuid();
        static std::string generate_uuid_v4();
};


#endif