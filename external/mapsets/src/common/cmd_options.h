#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <memory>
#include <cassert>

using namespace std;

class CMDOptions
{
private:
	string usageMessage;
	map<string, string> options;

	map<string, string> allowedOptions;
	vector<string> allowedOptionsOrder;
	map<string, string> defaultValues;
	map<string, vector<string> > allowedValues;

	CMDOptions(const CMDOptions&);
	CMDOptions& operator = (const CMDOptions&);
	CMDOptions() {}

public:
	static unique_ptr<CMDOptions> Create()
	{
		return unique_ptr<CMDOptions>(new CMDOptions());
	}

	void Parse(int argc, char **argv)
	{
		for (int i = 1; i < argc; i++)
		{
			string s(argv[i]);
			if (s == "/?"  || s == "-?" || s == "--help" || s == "-help")
			{
				Usage(argv[0]);
				throw 0;
			}

			SetOption(s);
		}
	}

	void SetUsageMessage(const string& msg)
	{
		usageMessage = msg;
	}

	void SetOption(const string& s)
	{
		size_t equalIndex = s.find('=');
		string name = s.substr(0, equalIndex);
		if (!allowedOptions.count(name))
		{
			if (equalIndex == string::npos && allowedOptions.count(""))
			{
				options[""] = name;
				return;
			}

			UnrecognizedOption(name);
		}

		string value = (equalIndex == string::npos ? "" : s.substr(equalIndex + 1));

		if (!options.count(name) || (defaultValues.count(name) && options[name] == defaultValues[name]))
			options[name] = value;

		if (!allowedValues[name].empty() && !count(allowedValues[name].begin(), allowedValues[name].end(), value))
			InvalidOption(name);
	}

	void AddAllowedOption(const string& optionName, const string& defaultValue, const string& description)
	{
		AddAllowedOption(optionName, description);
		options[optionName] = defaultValue;
		defaultValues[optionName] = defaultValue;
	}

	void AddAllowedOption(const string& optionName, const string& description)
	{
		assert(!allowedOptions.count(optionName));
		allowedOptions[optionName] = description;
		allowedOptionsOrder.push_back(optionName);
	}

	void AddAllowedValue(const string& optionName, const string& value)
	{
		assert(allowedOptions.count(optionName));
		allowedValues[optionName].push_back(value);
	}

	string getOption(const string& optionName) const
	{
		if (!options.count(optionName) && allowedOptions.count(optionName))
			UnspecifiedOption(optionName);

		assert(options.count(optionName));
		return (*options.find(optionName)).second;
	}

	bool hasOption(const string& optionName) const
	{
		if (!allowedOptions.count(optionName))
			UnrecognizedOption(optionName);

		return options.count(optionName) > 0;
	}

	void UnspecifiedOption(const string& optionName) const
	{
		cout << "required option \"" << optionName << "\" is not specified\n";
		throw 1;
	}

	void UnrecognizedOption(const string& optionName) const
	{
		cout << "unrecognized option \"" << optionName << "\"\n";
		throw 1;
	}

	void InvalidOption(const string& optionName) const
	{
		cout << "value \"" << getOption(optionName) << "\" is invalid for option \"" << optionName << "\"\n";
		throw 1;
	}

	void Usage(const string& program) const
	{
		if (usageMessage != "")
			cout << usageMessage << endl;
		else
			cout << "Usage: " << program << " [options]" << endl;

		cout << "Allowed options:";
		for (auto opt : allowedOptionsOrder)
		{
			string name = allowedOptions.find(opt)->first;
			if (name.length() == 0) continue;

			cout << endl;
			cout << "  " << name;
			if (allowedValues.count(name))
			{
				auto av = allowedValues.find(name)->second;
				if (!av.empty())
				{
					cout << "=";
					bool first = true;
					for (string s: av)
						if (first) 
							{ cout << "[" << s; first = false; }
						else 
							cout << "|" << s;
					cout << "]";
				}
			}
			cout << endl;

			cout << "  " << allowedOptions.find(opt)->second << endl;
		}
	}
};
