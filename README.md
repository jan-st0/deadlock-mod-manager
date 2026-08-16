# Deadlock Mod Manager

A lightweight C++ application for managing Deadlock game modifications.

## Overview

Deadlock Mod Manager provides a streamlined interface for organizing and deploying game mods. The application maintains an internal repository of `.vpk` files within a structured directory system, enabling efficient mod management and preset configurations.

## Directory Structure

```
mods/
├── [mod_name_1]/
│   └── *.vpk
├── [mod_name_2]/
│   └── *.vpk
└── ...
```

## Features

- **Mod Management**: Store and organize `.vpk` mod files in a dedicated directory structure
- **Preset System**: Create and save collections of mods for different gameplay configurations
- **Automatic Deployment**: Deploy mods by copying `.vpk` files to the Deadlock addons directory
- **Clean Installation**: Automatically remove excess files from the addons folder during deployment
- **Persistence**: Latest preset configuration saved to `last_preset.txt`

## Planned Enhancements

- **GameInfo Integration**: Custom `gameinfo.gi` generation with mod support
- **Field of View Configuration**: Direct FOV adjustment through `gameinfo.gi`
- **Visual Metadata**: Automatic image retrieval from GameBanana using the origin field

## Technical Implementation

- **Language**: C++
- **Mod Format**: `.vpk` (Valve Pak) files
- **Configuration**: Plain text preset files