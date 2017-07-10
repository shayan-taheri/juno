
#ifndef _H_SST_JUNO_LD_ST_UNIT
#define _H_SST_JUNO_LD_ST_UNIT

#include <sst/core/interfaces/simpleMem.h>
#include <map>

#include "junoregfile.h"

using namespace SST::Interfaces;

namespace SST {
namespace Juno {

class JunoLoadStoreEntry {

public:
	JunoLoadStoreEntry( const SimpleMem::Request::id_t reqID, uint8_t regTgt ) :
		id(reqID), regTarget(regTgt) {}

	~JunoLoadStoreEntry() {}

	uint8_t getRegister() { return regTarget; }
	SimpleMem::Request::id_t getID() { return id; }

protected:
	SimpleMem::Request::id_t id;
	uint8_t regTarget;

};

class JunoLoadStoreUnit {

public:
	JunoLoadStoreUnit( SST::Output* out, SimpleMem* smMem, JunoRegisterFile* rFile ) :
		output(out), mem(smMem), regFile(rFile) {}

	bool operationsPending() {
		return pending.size() > 0;
	}

	void createLoadRequest( uint64_t addr, uint8_t reg ) {
		output->verbose(CALL_INFO, 16, 0, "Creating a load from address: %" PRIu64 " into register: %" PRIu8 "\n",
			addr, reg);

		SimpleMem::Request* req = new SimpleMem::Request(SimpleMem::Request::Read, addr, 8);

		JunoLoadStoreEntry* entry = new JunoLoadStoreEntry( req->id, reg );
		addEntry( entry );

		mem->sendRequest( req );
	}

	void createStoreRequest( uint64_t addr, uint8_t reg ) {
		output->verbose(CALL_INFO, 16, 0, "Creating a store from register %" PRIu8 " to address: %" PRIu64 "...\n",
			reg, addr);

                SimpleMem::Request* req = new SimpleMem::Request(SimpleMem::Request::Write, addr, 8);

		std::vector<uint8_t> payload;
		payload.resize(8);
		int64_t regValue = regFile->readReg( reg );

		memcpy( (void*) &payload[0], (void*) &regValue, sizeof(regValue) );
		req->setPayload( payload );

                JunoLoadStoreEntry* entry = new JunoLoadStoreEntry( req->id, reg );
                addEntry( entry );

                mem->sendRequest( req );
        }

	void addEntry( JunoLoadStoreEntry* entry ) {
		pending.insert( std::pair<SimpleMem::Request::id_t, JunoLoadStoreEntry*>( entry->getID(), entry ) );
	}

	uint8_t lookupEntry( SimpleMem::Request::id_t id ) {
		auto entry = pending.find( id );

		if( entry == pending.end() ) {
			fprintf(stderr, "Error: response from memory could not be found.\n");
			exit(-1);
		}

		return entry->second->getRegister();
	}

	void removeEntry( SimpleMem::Request::id_t id ) {
		auto entry = pending.find( id );

		if( entry != pending.end() ) {
			pending.erase(entry);
		}
	}

private:
	SST::Output* output;
	SimpleMem* mem;
	JunoRegisterFile* regFile;
	std::map<SimpleMem::Request::id_t, JunoLoadStoreEntry*> pending;

};

}
}

#endif
