#pragma once
#ifndef _GREEKBOT_PAYLOAD_H_
#define _GREEKBOT_PAYLOAD_H_
#include "json.h"

enum ePayloadOpcode {
	PAYLOAD_READY         = 0,
	PAYLOAD_HELLO         = 10,
	PAYLOAD_HEARTBEAT_ACK = 11
};

class cHelloPayload final {
private:
	int m_hi;
	
public:
	cHelloPayload(const json::value& v) : m_hi(static_cast<int>(v.at("d").as_object().at("heartbeat_interval").as_int64())) {}
	
	int GetHeartbeatInterval() const { return m_hi; }
};
typedef const std::unique_ptr<cHelloPayload> hHelloPayload;


class cPayload final {
private:
	const json::value m_val;
	ePayloadOpcode m_op;
	
public:
	cPayload(const json::value& v) : m_op(static_cast<ePayloadOpcode>(v.at("op").as_int64())), m_val(v) {}
	
	ePayloadOpcode GetOpcode() const { return m_op; }
	
	hHelloPayload ToHelloPayload() {
		try {
			return std::make_unique<cHelloPayload>(m_val);
		}
		catch (const std::exception&) {
			return std::unique_ptr<cHelloPayload>();
		}
	}
	
};
typedef const std::unique_ptr<cPayload> hPayload;

#endif /* _GREEKBOT_PAYLOAD_H_ */
