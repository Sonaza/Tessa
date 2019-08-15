#pragma once

TS_PACKAGE1(lang)

class Noncopyable
{
public:
	Noncopyable() = default;

	// Delete copy constructor and assignment
	Noncopyable(const Noncopyable &) = delete;
	Noncopyable &operator=(const Noncopyable &) = delete;
};

class NoncopyableAndNonmovable
{
public:
	NoncopyableAndNonmovable() = default;

	// Delete copy constructor and assignment
	NoncopyableAndNonmovable(const NoncopyableAndNonmovable &) = delete;
	NoncopyableAndNonmovable &operator=(const NoncopyableAndNonmovable &) = delete;

	// Delete move constructor and assignment
	NoncopyableAndNonmovable(NoncopyableAndNonmovable &&) = delete;
	NoncopyableAndNonmovable &operator=(NoncopyableAndNonmovable &&) = delete;
};

TS_END_PACKAGE1()
