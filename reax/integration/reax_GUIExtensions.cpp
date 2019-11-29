using std::placeholders::_1;

ComponentExtension::ComponentExtension(Component& parent)
: colourSubjects(new std::map<int, PublishSubject<juce::Colour>>()),
  parent(parent),
  bounds(parent.getBounds()),
  visible(parent.isVisible()),
  disposeBag(new DisposeBag())
{
    parent.addComponentListener(this);

    bounds.skip(1).subscribe([&parent](const Rectangle<int>& bounds) {
                      parent.setBounds(bounds);
                  })
        .disposedBy(*disposeBag);

    visible.skip(1).subscribe(std::bind(&Component::setVisible, &parent, _1)).disposedBy(*disposeBag);
}

ComponentExtension::~ComponentExtension()
{
    parent.removeComponentListener(this);
}

Observer<Colour> ComponentExtension::colour(int colourId) const
{
    // Create subject, if not already done
    if (colourSubjects->find(colourId) == colourSubjects->end())
        colourSubjects->insert(std::make_pair(colourId, PublishSubject<Colour>()));

    // Subscribe
    colourSubjects->at(colourId).subscribe(std::bind(&Component::setColour, &parent, colourId, _1)).disposedBy(*disposeBag);

    // Return as Observer
    return colourSubjects->at(colourId);
}

void ComponentExtension::componentMovedOrResized(Component&, bool, bool)
{
    if (parent.getBounds() != bounds.getValue())
        bounds.onNext(parent.getBounds());
}

void ComponentExtension::componentVisibilityChanged(Component&)
{
    if (parent.isVisible() != visible.getValue())
        visible.onNext(parent.isVisible());
}


ButtonExtension::ButtonExtension(Button& parent)
: ComponentExtension(parent),
  clicked(_clicked),
  buttonState(parent.getState()),
  toggleState(parent.getToggleState()),
  text(_text),
  tooltip(_tooltip)
{
    parent.addListener(this);

    _text.subscribe(std::bind(&Button::setButtonText, &parent, _1)).disposedBy(disposeBag);
    _tooltip.subscribe(std::bind(&Button::setTooltip, &parent, _1)).disposedBy(disposeBag);
    buttonState.skip(1).subscribe(std::bind(&Button::setState, &parent, _1)).disposedBy(disposeBag);
    toggleState.skip(1).subscribe([&parent](bool toggled) {
                           parent.setToggleState(toggled, sendNotificationSync);
                       })
        .disposedBy(disposeBag);
}

ButtonExtension::~ButtonExtension()
{
    // The constructor has initialized ComponentExtension() with a Button& parent.
    static_cast<Button&>(parent).removeListener(this);
}

void ButtonExtension::buttonClicked(Button*)
{
    _clicked.onNext(Empty());
}

void ButtonExtension::buttonStateChanged(Button* button)
{
    if (button->getState() != buttonState.getValue())
        buttonState.onNext(button->getState());

    if (button->getToggleState() != toggleState.getValue())
        toggleState.onNext(button->getToggleState());
}

ImageComponentExtension::ImageComponentExtension(ImageComponent& parent)
: ComponentExtension(parent),
  image(_image),
  imagePlacement(_imagePlacement)
{
    _image.subscribe([&parent](const Image& image) {
              parent.setImage(image);
          })
        .disposedBy(disposeBag);

    _imagePlacement.subscribe(std::bind(&ImageComponent::setImagePlacement, &parent, _1)).disposedBy(disposeBag);
}

LabelExtension::LabelExtension(Label& parent)
: ComponentExtension(parent),
  _discardChangesWhenHidingEditor(false),
  _textEditor(parent.getCurrentTextEditor()),
  text(parent.getText()),
  showEditor(parent.getCurrentTextEditor() != nullptr),
  discardChangesWhenHidingEditor(_discardChangesWhenHidingEditor),
  font(_font),
  justificationType(_justificationType),
  borderSize(_borderSize),
  attachedComponent(_attachedComponent),
  attachedOnLeft(_attachedOnLeft),
  minimumHorizontalScale(_minimumHorizontalScale),
  keyboardType(_keyboardType),
  editableOnSingleClick(_editableOnSingleClick),
  editableOnDoubleClick(_editableOnDoubleClick),
  lossOfFocusDiscardsChanges(_lossOfFocusDiscardsChanges),
  textEditor(_textEditor.distinctUntilChanged())
{
    parent.addListener(this);

    text.skip(1).subscribe(std::bind(&Label::setText, &parent, _1, sendNotificationSync)).disposedBy(disposeBag);

    showEditor.skip(1).withLatestFrom(_discardChangesWhenHidingEditor).subscribe([&parent](const std::tuple<bool, bool>& tuple) {
                                                                          if (std::get<0>(tuple))
                                                                              parent.showEditor();
                                                                          else
                                                                              parent.hideEditor(std::get<1>(tuple));
                                                                      })
        .disposedBy(disposeBag);

    _font.subscribe(std::bind(&Label::setFont, &parent, _1)).disposedBy(disposeBag);
    _justificationType.subscribe(std::bind(&Label::setJustificationType, &parent, _1)).disposedBy(disposeBag);
    _borderSize.subscribe(std::bind(&Label::setBorderSize, &parent, _1)).disposedBy(disposeBag);

    _attachedComponent.subscribe([&parent](const WeakReference<Component>& component) {
                          parent.attachToComponent(component, parent.isAttachedOnLeft());
                      })
        .disposedBy(disposeBag);

    _attachedOnLeft.subscribe([&parent](bool attachedOnLeft) {
                       parent.attachToComponent(parent.getAttachedComponent(), attachedOnLeft);
                   })
        .disposedBy(disposeBag);

    _minimumHorizontalScale.subscribe(std::bind(&Label::setMinimumHorizontalScale, &parent, _1)).disposedBy(disposeBag);

    _keyboardType.subscribe([&parent](TextInputTarget::VirtualKeyboardType keyboardType) {
                     parent.setKeyboardType(keyboardType);

                     if (auto editor = parent.getCurrentTextEditor())
                         editor->setKeyboardType(keyboardType);
                 })
        .disposedBy(disposeBag);

    // Cannot use combineLatest for these, because changing something on the Slider directly doesn't update the subject
    _editableOnSingleClick.subscribe([&parent](bool editableOnSingleClick) {
                              parent.setEditable(editableOnSingleClick, parent.isEditableOnDoubleClick(), parent.doesLossOfFocusDiscardChanges());
                          })
        .disposedBy(disposeBag);
    _editableOnDoubleClick.subscribe([&parent](bool editableOnDoubleClick) {
                              parent.setEditable(parent.isEditableOnSingleClick(), editableOnDoubleClick, parent.doesLossOfFocusDiscardChanges());
                          })
        .disposedBy(disposeBag);
    _lossOfFocusDiscardsChanges.subscribe([&parent](bool lossOfFocusDiscardsChanges) {
                                   parent.setEditable(parent.isEditableOnSingleClick(), parent.isEditableOnDoubleClick(), lossOfFocusDiscardsChanges);
                               })
        .disposedBy(disposeBag);
}

LabelExtension::~LabelExtension()
{
    // The constructor has initialized ComponentExtension() with a Label& parent.
    static_cast<Label&>(parent).removeListener(this);
}

void LabelExtension::labelTextChanged(Label* parent)
{
    if (parent->getText() != text.getValue())
        text.onNext(parent->getText());
}

void LabelExtension::editorShown(Label* parent, TextEditor&)
{
    if (!showEditor.getValue())
        showEditor.onNext(true);

    _textEditor.onNext(parent->getCurrentTextEditor());
}

void LabelExtension::editorHidden(Label*, TextEditor&)
{
    if (showEditor.getValue())
        showEditor.onNext(false);

    _textEditor.onNext(nullptr);
}


SliderExtension::SliderExtension(juce::Slider& parent,
                                 const Observer<std::function<double(const juce::String&)>>& getValueFromText,
                                 const Observer<std::function<juce::String(double)>>& getTextFromValue)
: ComponentExtension(parent),
  _dragging(false),
  _discardChangesWhenHidingTextBox(false),
  value(parent.getValue()),
  minimum(_minimum),
  maximum(_maximum),
  minValue(hasMultipleThumbs(parent) ? parent.getMinValue() : parent.getValue()),
  maxValue(hasMultipleThumbs(parent) ? parent.getMaxValue() : parent.getValue()),
  doubleClickReturnValue(_doubleClickReturnValue),
  interval(_interval),
  skewFactorMidPoint(_skewFactorMidPoint),
  dragging(_dragging.distinctUntilChanged()),
  thumbBeingDragged(dragging.map([&parent](bool) { return parent.getThumbBeingDragged(); })),
  showTextBox(_showTextBox),
  textBoxIsEditable(_textBoxIsEditable),
  discardChangesWhenHidingTextBox(_discardChangesWhenHidingTextBox),
  getValueFromText(getValueFromText),
  getTextFromValue(getTextFromValue)
{
    parent.addListener(this);

    value.skip(1).subscribe([&parent](double value) {
                     parent.setValue(value, sendNotificationSync);
                 })
        .disposedBy(disposeBag);

    // Cannot use combineLatest for these, because changing something on the Slider directly doesn't update the subject
    _minimum.subscribe([&parent](double minimum) {
                parent.setRange(minimum, parent.getMaximum(), parent.getInterval());
            })
        .disposedBy(disposeBag);
    _maximum.subscribe([&parent](double maximum) {
                parent.setRange(parent.getMinimum(), maximum, parent.getInterval());
            })
        .disposedBy(disposeBag);
    _interval.subscribe([&parent](double interval) {
                 parent.setRange(parent.getMinimum(), parent.getMaximum(), interval);
             })
        .disposedBy(disposeBag);

    minValue.skip(1).subscribe([&parent](double minValue) {
                        parent.setMinValue(minValue, sendNotificationSync, true);
                    })
        .disposedBy(disposeBag);

    maxValue.skip(1).subscribe([&parent](double maxValue) {
                        parent.setMaxValue(maxValue, sendNotificationSync, true);
                    })
        .disposedBy(disposeBag);

    _doubleClickReturnValue.subscribe([&parent](double value) {
                               parent.setDoubleClickReturnValue(value != std::numeric_limits<double>::max(), value);
                           })
        .disposedBy(disposeBag);

    _skewFactorMidPoint.subscribe(std::bind(&Slider::setSkewFactorFromMidPoint, &parent, _1)).disposedBy(disposeBag);

    _showTextBox.withLatestFrom(_discardChangesWhenHidingTextBox).subscribe([&parent](const std::tuple<bool, bool>& tuple) {
                                                                     if (std::get<0>(tuple))
                                                                         parent.showTextBox();
                                                                     else
                                                                         parent.hideTextBox(std::get<1>(tuple));
                                                                 })
        .disposedBy(disposeBag);

    _textBoxIsEditable.subscribe(std::bind(&Slider::setTextBoxIsEditable, &parent, _1)).disposedBy(disposeBag);
}

SliderExtension::~SliderExtension()
{
    // The constructor has initialized ComponentExtension() with a Slider& parent.
    static_cast<Slider&>(parent).removeListener(this);
}

void SliderExtension::sliderValueChanged(Slider* slider)
{
    if (slider->getValue() != value.getValue())
        value.onNext(slider->getValue());

    if (hasMultipleThumbs(*slider) && slider->getMinValue() != minValue.getValue())
        minValue.onNext(slider->getMinValue());

    if (hasMultipleThumbs(*slider) && slider->getMaxValue() != maxValue.getValue())
        maxValue.onNext(slider->getMaxValue());
}

void SliderExtension::sliderDragStarted(Slider*)
{
    _dragging.onNext(true);
}

void SliderExtension::sliderDragEnded(Slider*)
{
    _dragging.onNext(false);
}

bool SliderExtension::hasMultipleThumbs(const juce::Slider& parent)
{
    switch (parent.getSliderStyle()) {
        case Slider::TwoValueHorizontal:
        case Slider::TwoValueVertical:
        case Slider::ThreeValueHorizontal:
        case Slider::ThreeValueVertical:
            return true;

        default:
            return false;
    }
}
