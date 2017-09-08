# CustomTabWidget
Experimentation on how it would be possible to create proper dock widgets that don't have the default Qt UX weirdness.
Problems with QDockWidget:

1. Can not add menu button on the right side of the tabs.

2. Can not have a docked widget with just one tab, instead it removes the tab which makes this look and function in a very inconsistent way.

3. Can not have a floating window that has only one tab.

4. Can not remove the functionality where the dockwidget moves and resizes all the widgets when dragging it.
	a. This makes it very difficult to dock when everything is moving around.
	b. This makes docking process very sluggish and not responsive enough.

5. The drag widget preview that is below the mouse cursor is blocking the view and makes it difficult to see where the widget will end up when you drop it.