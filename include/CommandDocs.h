#pragma once
#include <string>
#include <deque>
#include <vector>
#include <unordered_map>

struct CommandDoc
{
	std::string name;
	std::string summary;
	std::string since;
	std::string group;
	std::string complexity;
	std::deque<std::string> arguments;
};

class CommandDocs
{
public:
	CommandDocs() = delete;
	static void init();
	static const CommandDoc* get(std::string cmd);
	static std::deque<std::string> allNames();
	static int count();
	static std::vector<const CommandDoc*> lookup(const std::vector<std::string>& cmds);
private:
	static std::unordered_map<std::string, CommandDoc> m_docs;
};

