#ifndef DISCORD_VISITOR_H
#define DISCORD_VISITOR_H
/* ========== A helper class to combine multiple functors into a single visitor ===================================== */
template<typename... Fns>
struct cVisitor : Fns... {
	using Fns::operator()...;
};
#endif /* DISCORD_VISITOR_H */