{ipcMain, BrowserWindow} = require 'electron'
v8Util = process.atomBinding 'v8_util'

frameToGuest = {}

# Copy attribute of |parent| to |child| if it is not defined in |child|.
mergeOptions = (child, parent) ->
  for own key, value of parent when key not of child
    if typeof value is 'object'
      child[key] = mergeOptions {}, value
    else
      child[key] = value
  child

# Merge |options| with the |embedder|'s window's options.
mergeBrowserWindowOptions = (embedder, options) ->
  if embedder.browserWindowOptions?
    # Inherit the original options if it is a BrowserWindow.
    mergeOptions options, embedder.browserWindowOptions
  else
    # Or only inherit web-preferences if it is a webview.
    options.webPreferences ?= {}
    mergeOptions options.webPreferences, embedder.getWebPreferences()
  options

# Create a new guest created by |embedder| with |options|.
createGuest = (embedder, url, frameName, options) ->
  # TODO - figure out how to use either webcontents or guestInstanceId here
  guest = frameToGuest[frameName]
  if frameName and guest?
    guest.loadURL url
    return guest.id

  # Remember the embedder window's id.
  options.webPreferences ?= {}

  # send webContents as an option??
  guest = new BrowserWindow(options)
  guest.loadURL url

  # When |embedder| is destroyed we should also destroy attached guest, and if
  # guest is closed by user then we should prevent |embedder| from double
  # closing guest.
  guestId = guest.id
  closedByEmbedder = ->
    guest.removeListener 'closed', closedByUser
    guest.destroy()
  closedByUser = ->
    embedder.send "ATOM_SHELL_GUEST_WINDOW_MANAGER_WINDOW_CLOSED_#{guestId}"
    embedder.removeListener 'render-view-deleted', closedByEmbedder
  embedder.once 'render-view-deleted', closedByEmbedder
  guest.once 'closed', closedByUser

  if frameName
    frameToGuest[frameName] = guest
    guest.frameName = frameName
    guest.once 'closed', ->
      delete frameToGuest[frameName]

  guest.id

# Routed window.open messages.
process.on 'ATOM_SHELL_GUEST_WINDOW_MANAGER_WINDOW_OPEN', (event, args...) ->
  [url, frameName, options] = args
  options = mergeBrowserWindowOptions event.sender, options
  event.sender.emit 'new-window', event, url, frameName, 'new-window', options
  if (event.sender.isGuest() and not event.sender.allowPopups) or event.defaultPrevented
    event.returnValue = null
  else
    event.returnValue = createGuest event.sender, url, frameName, options
