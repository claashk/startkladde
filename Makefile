# The debug version is called startkladde_debug. The release version is called
# startkladde, not startkladde_release, due to qmake stinkage (see
# startkladde.pro).

# Note: the program (startkladde) is built by another Makefile which is
# generated by qmake. Any dependency of startkladde which is built by this
# Makefile has to be stated explicitly, even though it is also present in the
# generated Makefile. These dependencies are in the startkladde_autogenerated
# target.
# For example: startkladde depends on CurrentSchema.cpp which is autogenerated
# from current_schema.yaml by this Makefile.
# If current_schema.yaml is changed, CurrentSchema.cpp has to be rebuilt. This
# only happens if CurrentSchema.cpp is a prerequisite of startkladde in *this*
# Makefile. Also, the generated Makefile would not know how to build
# CurrentSchema.cpp

##############
## Settings ##
##############

# Set to qmake-qt4 if the default qmake is qmake-qt3 on this system
QMAKE = qmake -Wall
#QMAKE = qmake -Wall -spec ./config/mkspecs/win32-cross-mingw


####################
## Common targets ##
####################

.PHONY: default all startkladde
default: startkladde

startkladde: release

startkladde_debug: debug

all: startkladde #plugins


#########################
## Generic build rules ##
#########################

# A Makefile ist created from the corresponding project files by invoking qmake
# This will also generate the .Debug and .Release Makefiles if required by the
# .pro file.
Makefile_%: %.pro
	$(QMAKE) $<

# A .Debug or .Release Makefile is generated with the corresponding Makefile.
# Note that this rule requires a command (even if it is empty), or it will not
# work (i. e. the File will not be reubuilt if it is out of date.
Makefile_%.Debug Makefile_%.Release: Makefile_%
	#


########################
## Individual targets ##
########################

# The startkladde Makefiles depend on the migration list
Makefile_startkladde: build/migrations.pro


#########################
## Autogenerated files ##
#########################

# See the comment at the beginning of this file
#
# The following files are autogenerated (with template and dependencies):
#   * build/migrations.pro       (src/migrations.pro.erb                    - src/db/migrations)
#   * build/migrations.h         (src/db/migration/migrations.h.erb         - src/db/migrations)
#   * build/migrations_headers.h (src/db/migration/migrations_headers.h.erb - src/db/migrations)
#   * build/CurrentSchema.cpp    (src/db/schema/CurrentSchema.cpp.erb       - src/db/migrations/current_schema.yaml)
# All of these files are automatically generated by ./script/build/
# autogenerate_files.rb. Therefore, only one of them (build/migrations.pro, the
# one required for qmake) is listed here, with all templates as dependency.
# This should be changed: either add the files to version control, or update
# them one at a time

startkladde_autogenerated: build/migrations.pro version/version.h

templates=src/db/migration/migrations.h.erb src/db/migration/migrations_headers.h.erb src/db/schema/CurrentSchema.cpp.erb src/migrations.pro.erb

build/migrations.pro: $(templates) src/db/migrations src/db/migrations/current_schema.yaml
	./script/build/autogenerate_files.rb

version/version.h: version/version version/make_version
	cd version; ./make_version; cd ..



####################
## Subdirectories ##
####################

#.PHONY: plugins
#plugins:
#	$(MAKE) -C $@


#############
## Cleanup ##
#############

.PHONY: clean
clean: Makefile_startkladde
	$(MAKE) -f Makefile_startkladde distclean
	rm -f Makefile_startkladde Makefile_startkladde.Debug Makefile_startkladde.Release
	rm -f version/version.h
	rm -rf build debug release
	rm -f object_script.startkladde* # For Windows
	rm -f startkladde startkladde_release startkladde_debug
#	$(MAKE) -C plugins clean


##########
## Misc ##
##########

.PHONY: run run-debug
run: startkladde
	./startkladde -q --no-full-screen

run-debug: startkladde_debug
	./startkladde_debug -q --no-full-screen

# Use a temporary file in build/, so if the dumping fails, we don't overwrite
# the schema
.PHONY: update_current_schema
update_current_schema: startkladde_debug
	mkdir -p build
	./startkladde_debug db:ensure_empty || (echo "Error: database is not empty - use ./startkladde db:clear"; false)
	./startkladde_debug db:migrate
	./startkladde_debug db:dump build/current_schema.yaml
	mv build/current_schema.yaml src/db/migrations/current_schema.yaml
	echo "Schema definition updated successfully"

# Forward to Makefile_startkladde
.PHONY: release debug install uninstall
release debug install uninstall: Makefile_startkladde startkladde_autogenerated
	INSTALL_ROOT=${DESTDIR} $(MAKE) -f $< $@

debug-% release-%: Makefile_startkladde startkladde_autogenerated
    INSTALL_ROOT=${DESTDIR} $(MAKE) -f $< $@
