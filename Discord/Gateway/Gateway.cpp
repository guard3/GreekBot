#include "GatewayImpl.h"

cGateway::cGateway(const char* t, eIntent i) : m_pImpl(cHandle::MakeUnique<implementation>(this, t, i)) {}
cGateway::~cGateway() = default;

const char*
cGateway::GetHttpAuthorization() const noexcept { return m_pImpl->GetHttpAuthorization(); }
const char*
cGateway::GetToken() const noexcept { return m_pImpl->GetToken(); }
void
cGateway::Run() { m_pImpl->Run(); }