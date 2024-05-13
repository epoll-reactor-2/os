-- jumptag - Jump to any function, class or heading with F4. Go, Python, C...
-- Copyright 2020-2021 Tero Karvinen http://TeroKarvinen.com
-- https://github.com/terokarvinen/micro-jump
-- MIT license

local config = import("micro/config")
local buffer = import("micro/buffer")
local shell = import("micro/shell")
local micro = import("micro")

function init()
	config.MakeCommand("jumptag", jumptagCommand, config.NoComplete)
	config.TryBindKey("F4", "command:jumptag", true)
end

function jumptagCommand(bp) -- bp bufPlane
    local cmd = "bash -c \"ctags -w -R -f - --fields=n '.' | fzf --layout=reverse | sed -E 's/\\s+/ /g;s/line://g' | awk '{print $2,$NF}'\""
    local out = shell.RunInteractiveShell(cmd, false, true)

    if out == "" or out == "nil" then return end

    local filename, lineno = out:match("(%S+)%s+(%S+)")

    micro.InfoBar():Message(string.format("Selected file %s:%s", filename, lineno))

    micro.CurPane():VSplitIndex(buffer.NewBufferFromFile(filename), true)
    micro.CurPane().Cursor.Y = tonumber(lineno) - 1
end