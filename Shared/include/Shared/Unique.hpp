#pragma once

// Inherit from this to disallow copying
class Unique
{
public:
	Unique() = default;
	Unique(const Unique& rhs) = delete;
	Unique& operator=(const Unique& rhs) = delete;
};
