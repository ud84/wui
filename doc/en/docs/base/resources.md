# Resources

Themes and locales are JSON data loaded by the application. Start with a complete
`examples/demo/res` or `examples/hello_world/res` directory, including images.
Small theme fragments in the reference describe individual controls; wrap them
in a `controls` array and provide the other sections required by your application.

| Platform | Example resource location |
| --- | --- |
| Windows | Embedded `.rc` resources; image theme uses the `resource` section name |
| Linux | `res/` beside the example executable |
| macOS | Bundle `Contents/Resources/res`; working-directory paths are tried first |
| WASM | Preloaded virtual `/res`, downloaded from the accompanying `.data` file |

An image theme's `path` locates files on non-Windows backends. Use explicit paths;
do not assume shell `~` expansion inside JSON. Keep `.js`, `.wasm` and `.data`
from the same build when publishing a browser application.

After loading a new theme, call `window->update_theme()`. After loading a locale,
update application captions with `set_text()`/`set_caption()`; existing strings
are not translated automatically. See [themes](theme.md) and [locales](locale.md).

Configuration is separate from read-only resources. `create_config()` uses the
registry on Windows and an INI file elsewhere. Choose a writable path on desktop;
WASM's filesystem is in memory and settings reset when the page reloads.
