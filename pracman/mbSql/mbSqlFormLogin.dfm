object FormLogin: TFormLogin
  Left = 445
  Top = 341
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Login '
  ClientHeight = 106
  ClientWidth = 179
  Color = clBtnFace
  Constraints.MaxHeight = 133
  Constraints.MinHeight = 133
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  Position = poScreenCenter
  OnActivate = FormActivate
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object LabelUsername: TLabel
    Left = 8
    Top = 8
    Width = 57
    Height = 19
    Alignment = taRightJustify
    AutoSize = False
    Caption = 'User Name'
    Layout = tlCenter
  end
  object LabelPassword: TLabel
    Left = 8
    Top = 40
    Width = 57
    Height = 19
    Alignment = taRightJustify
    AutoSize = False
    Caption = 'Password'
    Layout = tlCenter
  end
  object EditUsername: TEdit
    Left = 72
    Top = 8
    Width = 97
    Height = 21
    MaxLength = 32
    TabOrder = 0
  end
  object EditPassword: TEdit
    Left = 72
    Top = 40
    Width = 97
    Height = 21
    Font.Charset = SYMBOL_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Symbol'
    Font.Style = []
    MaxLength = 32
    ParentFont = False
    PasswordChar = #183
    TabOrder = 1
    OnKeyDown = EditPasswordKeyDown
  end
  object ButtonLogin: TButton
    Left = 112
    Top = 72
    Width = 59
    Height = 25
    Caption = 'Login'
    TabOrder = 2
    OnClick = ButtonLoginClick
  end
end
