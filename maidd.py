#!/usr/bin/env python

from subprocess import call, Popen, PIPE
from datetime import datetime
from time import sleep
import re

interval = 5 # seconds between forced standby, and timeouts for IO
apm_timeout = 30 # seconds before device will put itself to sleep
sleep_timeout = 11 # seconds till disk is put to sleep
disks = ['/dev/sdb']#, '/dev/sdb']

def standby_disk(disk):
	cmds = [('hdparm -y %s' % disk, 'put disk into standby mode (%s)' % disk)]
	run_cmds(cmds)

def sleep_disk(disk):
	cmds = [('hdparm -Y %s' % disk, 'put disk into sleep mode (%s)' % disk)]
	run_cmds(cmds)

def tune_disk(disk):
	cmds = [
		('hdparm -B 1 %s',					'set drive APM level'),
		('hdparm -S %d %%s' % apm_timeout,	'set drive sleep timeout'),
	]
	cmds = [(cmd[0] % disk, cmd[1] + ' (%s)' % disk) for cmd in cmds]
	run_cmds(cmds)

def get_state(disk):
	regex = 'drive state is:\s+(.*)\n'
	cmd = 'hdparm -C %s' % disk
	output = Popen(cmd.split(), stdout=PIPE).communicate()[0]
	m = re.search(regex, output)
	if m == None:
		return 'unknown (error)'
	else:
		return m.groups()[0]

def run_cmds(cmds):
	for cmd in cmds:
		if not call(cmd[0].split()) == 0:
			print '[i] could not %s' % cmd[1]
		else:
			print '[*] successfully %s' % cmd[1]

def main():
	asleep_times = {}
	for disk in disks:
		print '--- tuning %s for optimum idle times' % disk
		tune_disk(disk)
		asleep_times[disk] = None
	while True:
		sleep(interval)
		print '--- [%s] Checking disk states ---' % (datetime.now().isoformat())
		for disk in disks:
			state = get_state(disk)
			print '[i] disk %s in state: %s' % (disk, state)
			if state.startswith('sleeping'):
				print '[i] %s already in lowest power mode'
			if state.startswith('standby'):
				if asleep_times[disk] == None:
					asleep_times[disk] = datetime.now()
					print '[i] %s will go to sleep in %d seconds' % (disk, sleep_timeout)
				elif (datetime.now() - asleep_times[disk]).seconds > sleep_timeout:
					sleep_disk(disk)
				else:
					print '[i] %s will go to sleep in %d seconds' % (disk, sleep_timeout - (datetime.now() - asleep_times[disk]).seconds)
			elif state.startswith('active/idle'):
				if asleep_times[disk] != None:
					print '[i] %s woke up before sleep timeout' % disk
					asleep_times[disk] = None
				standby_disk(disk)
			else:
				print '[i] %s has unknown state - %s' % (disk, state)
		print

if __name__=='__main__':
	main()
