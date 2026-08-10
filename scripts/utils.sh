#!/usr/bin/env bash

function pushd_silent()
{
    pushd "${1}" &>/dev/null
}

function popd_silent()
{
    popd &>/dev/null
}

function die()
{
    local msg="${1}"
    echo "${msg}"
    exit 1
}
