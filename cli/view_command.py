# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & University of Surrey for the benefit of the
# BioDynaMo collaboration. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional new_steprmation
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

import os
import subprocess as sp
import sys

from print_command import Print


## The BioDynaMo CLI command to execute the unit-tests created by default with
## the simulation template. Note that we ignore the configuration in bdm.json.
def ViewCommand(file_id=None):
    Print.success("<bdm view> Opening previous simulation results ...")
    # 1. Check if ParaView is available on PATH.
    try:
        pv_found = sp.run(
            ["paraview", "--version"],
            stdout=sp.DEVNULL,
            stderr=sp.DEVNULL,
            check=False,
        )
    except FileNotFoundError:
        Print.error("<bdm view> ParaView not found.")
        sys.exit(1)
    if pv_found.returncode != 0:
        Print.error("<bdm view> ParaView could not be started.")
        sys.exit(1)

    # 2. attempt to change into project director if we are in build
    cwd = os.getcwd()
    if cwd.split("/")[-1] == "build":
        os.chdir("..")

    # 3. Find all pvsm files in output and build/output
    pvsm_files = []
    if os.path.isdir("output"):
        for root, dirs, files in os.walk("output"):
            for file in files:
                if file.endswith(".pvsm"):
                    pvsm_files.append(os.path.join(root, file))
    if os.path.isdir("build/output"):
        for root, dirs, files in os.walk("build/output"):
            for file in files:
                if file.endswith(".pvsm"):
                    pvsm_files.append(os.path.join(root, file))

    # 5. Verify if there are any pvsm files; and that file_id is valid
    if len(pvsm_files) == 0:
        Print.error("<bdm view> No pvsm files found.")
        sys.exit(1)

    # 6. Convert all pvsm files to absolute paths
    pvsm_files = [os.path.abspath(file) for file in pvsm_files]
    Print.new_step("<bdm view> Available PVSM files:")
    for i, file in enumerate(pvsm_files):
        print("<bdm view> File " + str(i + 1) + " : " + file)

    # 7. Choose the pvsm file
    Print.new_step("<bdm view> Selecting pvsm file.")
    if file_id is not None and file_id > len(pvsm_files):
        Print.error("<bdm view> Invalid file id ({}).".format(file_id))
        sys.exit(1)
    if file_id is None:
        print("<bdm view> By default, the latest pvsm file is opened.")
        print("<bdm view> Use 'bdm view <id>' to open another pvsm file.")
        pvsm_file = max(pvsm_files, key=os.path.getctime)
    else:
        pvsm_file = pvsm_files[file_id - 1]

    # 8. If the latest pvsm file is in the build folder,
    #    change to the build folder
    Print.new_step("<bdm view> Opening :" + pvsm_file)
    if "build" in pvsm_file:
        os.chdir("build")

    # 9. Open the latest pvsm file
    visualize = sp.run(["paraview", "--state={}".format(pvsm_file)])
    if visualize.returncode != 0:
        Print.warning(
            "<bdm view> Visualization terminated with return code {}.".format(
                visualize.returncode
            )
        )

    # 10. go back to initial folder.
    os.chdir(cwd)
    Print.success("<bdm view> Finished successfully.")
