import sys
import os
import re
import lldb
#from pprint import pprint

class EnvPyFormatException(Exception):
	pass

env_array = []

with open(os.path.join(os.environ['ENV_PY']))									as file:
	for																	line	in file:
		if not		re.match("^\s*(#.*)?$"							,	line):
			if m :=	re.match("^\s*(?P<key>\S+)\s*=(?P<value>.*)$"	,	line):
				env_array.append(m.group('key')+"="+m.group('value'))
			else:
				raise EnvPyFormatException("Line doesn't match ` `, ` # comment` or ` key =value` where space is optional:"
					,													line)

target = lldb.debugger.GetSelectedTarget()

launch_info = target.GetLaunchInfo()
launch_info.SetEnvironmentEntries(env_array, True)
target.SetLaunchInfo(launch_info)
