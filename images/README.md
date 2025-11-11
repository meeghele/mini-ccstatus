
# mini-ccstatus Demo

## Demo

<img src="mini-ccstatus_demo.gif" width="80%" alt="mini-ccstatus demo">

## `--all`

![mini-ccstatus all](images/mini-ccstatus_all.png)

```json
{
  "statusLine": {
    "type": "command",
    "command": "/path/to/mini-ccstatus/bin/mini-ccstatus --all",
    "padding": 0
  }
}
```

## Custom

![mini-ccstatus custom](images/mini-ccstatus_custom.png)

```json
{
  "statusLine": {
    "type": "command",
    "command": "/path/to/mini-ccstatus/bin/mini-ccstatus --simple --context-tokens --session-tokens --api-time-ratio --input-output-ratio",
    "padding": 0
  }
}
```

## Minimal

![mini-ccstatus default](images/mini-ccstatus_default.png)

```json
{
  "statusLine": {
    "type": "command",
    "command": "/path/to/mini-ccstatus/bin/mini-ccstatus",
    "padding": 0
  }
}
```
