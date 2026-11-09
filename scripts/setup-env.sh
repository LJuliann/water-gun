#!/usr/bin/env bash
set -euo pipefail

# Dépendances locales pour Linux x86_64, sans installation système.
project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
tools_dir="$project_dir/.tools"
sdk_commit=079c6f39023649b154152db30f1d781e884879bc

if [[ "$(uname -s)-$(uname -m)" != Linux-x86_64 ]]; then
    echo "Ce script cible Linux x86_64. Voir le README pour une installation manuelle." >&2
    exit 1
fi
for tool in cmake ninja git curl tar xz sha256sum python3 g++; do
    if ! command -v "$tool" >/dev/null; then
        echo "Prérequis manquant : $tool." >&2
        exit 1
    fi
done

mkdir -p "$tools_dir"
download_dir="$(mktemp -d "$tools_dir/download.XXXXXX")"
trap 'rm -rf -- "$download_dir"' EXIT

install_archive() {
    local name="$1" url="$2" checksum="$3" archive="$download_dir/$1.archive"
    if [[ -e "$tools_dir/$name" ]]; then
        echo "Déjà présent : $tools_dir/$name"
        return
    fi
    curl --fail --location --retry 3 --connect-timeout 20 "$url" -o "$archive"
    printf '%s  %s\n' "$checksum" "$archive" | sha256sum --check -
    mkdir "$download_dir/$name"
    tar -xf "$archive" -C "$download_dir/$name" --strip-components=1
    mv "$download_dir/$name" "$tools_dir/$name"
}

install_archive arm-toolchain \
    https://developer.arm.com/-/media/Files/downloads/gnu/15.2.rel1/binrel/arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi.tar.xz \
    597893282ac8c6ab1a4073977f2362990184599643b4c5ee34870a8215783a16
install_archive picotool \
    https://github.com/raspberrypi/pico-sdk-tools/releases/download/v2.3.1-0/picotool-2.3.1-x86_64-lin.tar.gz \
    878a1a1e7d8336b8be60300c7218f366d9790c0d3a211b181d6d172e80b1248e

if [[ ! -e "$tools_dir/pico-sdk" ]]; then
    git clone --depth 1 --branch 2.3.1 \
        https://github.com/raspberrypi/pico-sdk.git "$download_dir/pico-sdk"
    mv "$download_dir/pico-sdk" "$tools_dir/pico-sdk"
fi
if [[ "$(git -C "$tools_dir/pico-sdk" rev-parse HEAD)" != "$sdk_commit" ]]; then
    echo "Le SDK local ne correspond pas à la version 2.3.1 attendue." >&2
    exit 1
fi
git -C "$tools_dir/pico-sdk" submodule update --init --recursive --depth 1
if [[ -e "$project_dir/.git" ]]; then
    git -C "$project_dir" submodule update --init --recursive -- FreeRTOS-Kernel
elif [[ ! -f "$project_dir/FreeRTOS-Kernel/include/FreeRTOS.h" ]]; then
    echo "FreeRTOS-Kernel manque : utiliser le ZIP complet ou cloner le dépôt avec ses sous-modules." >&2
    exit 1
fi

"$tools_dir/arm-toolchain/bin/arm-none-eabi-gcc" --version
"$tools_dir/picotool/picotool" version
cd "$project_dir"
cmake --preset pico2
cmake --build --preset pico2
printf '\nEnvironnement prêt. Firmware : %s/build/water_gun.uf2\n' "$project_dir"
