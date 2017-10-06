#!/usr/bin/env python3

'''I know, it's awful... but it works, and without
    all the palava of autotools et. al.'''

import os
import yaml


class TPL:
    PREAMBLE = '''
CC ?= cc

TOPDIR = {topdir}
SRCDIR = {srcdir}
BUILDDIR = {builddir}
CFLAGS = -MMD -I$(SRCDIR)
LDFLAGS = -L$(BUILDDIR)

all: {targets}
clean:
{tab}rm -rvf $(BUILDDIR)
{tab}@echo '# reset makefile'
{tab}@echo '' >Makefile
{tab}@echo 'Makefile: project.yml' >>Makefile
{tab}@echo '	tools/genmk.py' >>Makefile
{tab}touch project.yml
.PHONY: all clean

Makefile: $(TOPDIR)/project.yml $(TOPDIR)/conf.yml $(TOPDIR)/tools/genmk.py
{tab}$(TOPDIR)/tools/genmk.py
'''

    TARGET = '''
{name}_OUTNAME = {t[out]}
{name}_OUT = $(BUILDDIR)/$({name}_OUTNAME)
{name}_DEP = {t[dep]}
{name}_DEPFILES = {t[depfiles]}
{name}_OUTDIR = {t[outdir]}
{name}_SRCDIR = {t[srcdir]}
{name}_FILES = {t[files]}
{name}_C = $({name}_FILES:%=$({name}_SRCDIR)/%.c)
{name}_O = $({name}_FILES:%=$({name}_OUTDIR)/%.o)
{name}_D = $({name}_O:.o=.d)
{name}_CFLAGS = {t[cflags]}
{name}_LDFLAGS = {t[ldflags]}
{name}_LDLIBS = {t[ldlibs]}

{name}: {name}_DIRS $({name}_DEP) $({name}_OUT)
{name}_DIRS:
{tab}mkdir -pv $(BUILDDIR) $({name}_OUTDIR) {t[subdirs]}
.PHONY: {name} {name}_DIRS

$({name}_OUT): $({name}_O) $({name}_DEPFILES)
{tab}$(CC) $({name}_LDFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS) $({name}_LDLIBS)

$({name}_OUTDIR)/%.o: $({name}_SRCDIR)/%.c
{tab}$(CC) $({name}_CFLAGS) $(CFLAGS) -c -o $@ $<

-include $({name}_D)
'''

    TARGET_NAME = '''
{name}: $({name}_OUT)
.PHONY: {name}
'''


while not os.path.isfile('project.yml'):
    if os.getcwd() == '/':
        raise Exception('No project!')
    os.chdir('..')


with open('project.yml', 'r') as rf:
    project = yaml.load(rf)


tab = '\t'
topdir = '.'
srcdir = 'src'
builddir = 'build'

with open(os.path.join('Makefile'), 'w') as wf:
    targets = ' '.join(target['name'] for target in project['targets'])
    wf.write(TPL.PREAMBLE.format(**locals()))

    for target in project['targets']:
        name = target['name']
        toutdir = os.path.join(builddir, 'out', name)
        t = {
            'srcdir': os.path.join(srcdir, name),
            'outdir': toutdir,
            'files': ' '.join(target['sources']),
            'subdirs': ' '.join(os.path.join(toutdir, subdir)
                                for subdir in target.get('subdirs', [])),
            'out': target.get('out', name),
            'dep': ' '.join(target.get('depend', [])),
            'depfiles': ' '.join('$(%s_OUT)' % dep
                                 for dep in target.get('depend', [])),
            'cflags': ' '.join(target.get('cflags', [])),
            'ldflags': ' '.join(target.get('ldflags', [])),
            'ldlibs': ' '.join(target.get('ldlibs', []))
        }
        wf.write(TPL.TARGET.format(**locals()))
        if t['out'] != name:
            wf.write(TPL.TARGET_NAME.format(**locals()))
