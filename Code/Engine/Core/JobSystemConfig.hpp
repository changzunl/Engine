#pragma once

struct JobSystemConfig
{
public:
	JobSystemConfig();
	~JobSystemConfig();

public:
	int m_workers = 3; // default for a 4-core
};

