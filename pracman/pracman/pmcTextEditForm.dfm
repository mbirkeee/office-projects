object TextEditForm: TTextEditForm
  Left = 582
  Top = 230
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'TextEditForm'
  ClientHeight = 72
  ClientWidth = 433
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object Label: TLabel
    Left = 8
    Top = 8
    Width = 80
    Height = 19
    Alignment = taRightJustify
    AutoSize = False
    Caption = 'Label'
    Layout = tlCenter
  end
  object TextEdit: TEdit
    Left = 96
    Top = 8
    Width = 329
    Height = 21
    MaxLength = 128
    TabOrder = 0
    Text = 'TextEdit'
    OnKeyDown = TextEditKeyDown
  end
  object CancelButton: TButton
    Left = 352
    Top = 40
    Width = 75
    Height = 25
    Caption = 'Cancel'
    TabOrder = 1
    OnClick = CancelButtonClick
  end
  object OkButton: TButton
    Left = 264
    Top = 40
    Width = 75
    Height = 25
    Caption = 'Ok'
    TabOrder = 2
    OnClick = OkButtonClick
  end
end
