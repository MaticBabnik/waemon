# Waemon

Wayland wallpaper daemon.

## Configuration

On startup configuration is read from one of (in order):

- `./waemon.jsonc`
- `${XDG_CONFIG_HOME}/waemon.jsonc`
- `~/.config/waemon.jsonc`
- `/etc/waemon.jsonc`

```jsonc
{
    "groups": {
        // Group names are used when sending commands over IPC

        "main": {
            "outputs": ["DP-3", "DP-2"],    // list of output names
            "backgroundColor": "#ffffff",   // (optional) hex color string like #123 or #abcdef
            "wallpaper": "/path/to/image",  // (optional) absolute path to image
            "mode": "zoom"                  // (optional) center | zoom | stretch | tile | contain
        },

        "secondary": {
            "outputs": ["HDMI-1"],
            "backgroundColor": "#0f0"
        },
    }
}
```

## Socket protocol [not functional]

### Get current status

Request

```json
["status"]
```

Response

```json
{
    "status": "ok",
    "groups": {
        "main": {
            "backgroundColor": "#ffffff",
            "wallpaper": "/path/to/image",
            "mode": "zoom",
            "total_w": 2560,
            "total_h": 1400,
            "matchedOutputs": [
                {
                    "name": "DP-2",
                    "x": 0,
                    "y": 0,
                    "w": 2560,
                    "h": 1440
                }
            ]
        },
    }
}
```

### Update group config

Request

```json
[
    "update", "<groupName>", {
        "backgroundColor": "#ffffff",
        "wallpaper": "/path/to/image",
        "mode": "zoom",
    }
]
```

(any subset of keys are valid, like in the config file)

Response

```json
{"status": "ok"}
```

### Errors

If anything goes wrong the daemon will reply with

```json
{"status": "error", "error": "womp womp"}
```

## Todo

- [x] static output group definitions (jsonc)
- [x] fixup WallpaperImage error handling and add cache
- [x] dynamic wallpaper and spantype changes over DBus or normal socket (json messages)
- [ ] split into daemon and cli
- [x] Come up with a new name (was paper)
- [ ] package for arch
