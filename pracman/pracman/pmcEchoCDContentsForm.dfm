object EchoCDContentsForm: TEchoCDContentsForm
  Left = 550
  Top = 181
  Width = 193
  Height = 148
  BorderIcons = [biSystemMenu]
  Caption = 'Select Echo CD'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object RadioGroup: TRadioGroup
    Left = 8
    Top = 8
    Width = 169
    Height = 73
    ItemIndex = 0
    Items.Strings = (
      'Echo CD #'
      'Check CD in drive')
    TabOrder = 1
    OnClick = RadioGroupClick
  end
  object Edit: TEdit
    Left = 112
    Top = 24
    Width = 49
    Height = 21
    MaxLength = 16
    TabOrder = 0
    OnKeyDown = EditKeyDown
  end
  object CancelButton: TButton
    Left = 96
    Top = 88
    Width = 81
    Height = 25
    Caption = 'Close'
    TabOrder = 2
    OnClick = CancelButtonClick
  end
  object CheckButton: TButton
    Left = 8
    Top = 88
    Width = 81
    Height = 25
    Caption = 'Check'
    TabOrder = 3
    OnClick = CheckButtonClick
  end
end
