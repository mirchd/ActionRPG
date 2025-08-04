# LuaMachine UMG

Requires LuaMachine 'dev' branch

```lua
print("Hello World")
user_widget = ui.create_user_widget()
vertical_box = user_widget.CreateVerticalBox()
button = user_widget.CreateButton()
border = user_widget.CreateBorder()
slot = border.SetContent(vertical_box)
slot.Padding = {Left=400, Right=100, Top=100, Bottom=100}
print('BORDER SLOT', slot.Padding.Left)
border.BrushColor = { R=1, G=0, B=1, A=0.3}
user_widget.SetRoot(border)
button.SetContent(user_widget.CreateCircularThrobber())
vertical_box.AddChild(button)
vertical_box.AddChild(user_widget.CreateSpacer())
check_box = user_widget.CreateCheckBox()
vertical_box.AddChild(check_box)
check_box.OnCheckStateChanged = function()
  print('CHECKED', check_box.CheckedState)
end

vertical_box.AddChild(user_widget.CreateSpacer())
vertical_box.AddChild(user_widget.CreateEditableText())
vertical_box.AddChild(user_widget.CreateSpacer())
image = user_widget.CreateImage()
image.ColorAndOpacity = {R=1, G=0, B=0, A=1}
brush = ui.load_texture_as_brush('/Game/GodOfWar2')
print('BRUSH', brush)
image.Brush = brush
slot = vertical_box.AddChild(image)
--slot.Padding = {Left=300, Right=100, Top=100, Bottom=200}
slot.Size = {Value=0.01, SizeRule=1}
color_and_opacity = image.ColorAndOpacity
print(color_and_opacity)
vertical_box.AddChild(user_widget.CreateSpacer())
text_block = user_widget.CreateTextBlock()
text_block.Text = "Hello World2"
slot = vertical_box.AddChild(text_block)
button.OnClicked = function()
  print("Button pressed")
  text_block.Text = os.time()
  print("Button pressed2")
  image.ColorAndOpacity = {R=math.random(), G=math.random(), B=math.random(), A=1}
end

button.OnHovered = function()
  print('hover!')
end


slot.Padding = {Left=300, Right=100, Top=100, Bottom=200}
```

```lua
user_widget = ui.create_user_widget()
canvas = user_widget.CreateCanvasPanel()
user_widget.SetRoot(canvas)


-- image
image = user_widget.CreateImage()
image.Brush = ui.load_texture_as_brush('/Game/GodOfWar2')

button = user_widget.CreateButton()
button.SetContent(image)
image_slot = canvas.AddChild(button)

image_slot.bAutoSize = true
layout_data = image_slot.LayoutData

layout_data.Alignment= { X=0.5, Y=0.5}
layout_data.Anchors.Minimum = { X=0.5, Y=0.5 }
layout_data.Anchors.Maximum = { X=0.5, Y=0.5 }

image_slot.LayoutData = layout_data

image.OnMouseButtonDownEvent = function()
    print("Hey!")
end

button.OnClicked = function()
    print("Clicked!")
end

canvas.AddChild(user_widget.CreateProgressBar())
```
