object Form_PickList: TForm_PickList
  Left = 425
  Top = 224
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'Form_PickList'
  ClientHeight = 92
  ClientWidth = 308
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object ComboBox: TComboBox
    Left = 8
    Top = 16
    Width = 291
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 0
  end
  object Button_OK: TButton
    Left = 224
    Top = 56
    Width = 75
    Height = 25
    Caption = 'OK'
    TabOrder = 1
    OnClick = Button_OKClick
  end
end
