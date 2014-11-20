#pragma once

#include "common/common.h"

class CMDOptions
{
	map<string, string> options;
	map<string, string> allowedOptions;

	CMDOptions(const CMDOptions&);
	CMDOptions& operator = (const CMDOptions&);

public:
	CMDOptions() {}

	void Parse(int argc, char **argv)
	{
		for (int i = 1; i < argc; i++)
		{
			string s(argv[i]);
			if (s == "/?"  || s == "-?" || s == "--help" || s == "-help")
			{
				Usage(argv[0]);
				exit(0);
			}

			vector<string> tmp = SplitNotNull(s, "=");
			if (tmp.size() == 1)
			{
				options[tmp[0]] = "";
			}
			else if (tmp.size() == 2)
			{
				options[tmp[0]] = tmp[1];
			}
			else
			{
				UnrecognizedOption(s);
			}
		}
	}

	void AddAllowedOption(const string& optionName, const string& defaultValue, const string& description)
	{
		AddAllowedOption(optionName, description);
		options[optionName] = defaultValue;
	}

	void AddAllowedOption(const string& optionName, const string& description)
	{
		assert(!allowedOptions.count(optionName));
		allowedOptions[optionName] = description;
	}

	string getOption(const string& optionName) const
	{
		if (!options.count(optionName) && allowedOptions.count(optionName))
			UnspecifiedOption(optionName);

		assert(options.count(optionName));
		return (*options.find(optionName)).second;
	}

	void UnspecifiedOption(const string& optionName) const
	{
		cout << "required option \"" << optionName << "\" is not specified\n";
		exit(1);
	}

	void UnrecognizedOption(const string& optionName) const
	{
		cout << "unrecognized option \"" << optionName << "\"\n";
		exit(1);
	}

	void InvalidOption(const string& optionName) const
	{
		cout << "value \"" << getOption(optionName) << "\" is invalid for option \"" << optionName << "\"\n";
		exit(1);
	}

	void Usage(const string& program) const
	{
		cout << "Usage: " << program << " [options]\n";
		cout << "Allowed options:\n";

		for (auto it = begin(allowedOptions); it != end(allowedOptions); it++)
		{
			cout << (*it).second << "\n";
		}
	}
};
