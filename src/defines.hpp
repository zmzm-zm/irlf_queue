#pragma once
/**
 *	@brief protect myself
 *	@details in order to keep my fingers and eyes alive
 *	@{
 */
constexpr auto release = std::memory_order_release;
constexpr auto acquire = std::memory_order_acquire;
constexpr auto relaxed = std::memory_order_relaxed;
/** @} */