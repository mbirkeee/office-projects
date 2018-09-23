object EchoPatSelectForm: TEchoPatSelectForm
  Left = 255
  Top = 204
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'EchoPatSelectForm'
  ClientHeight = 558
  ClientWidth = 347
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 65
    Height = 13
    AutoSize = False
    Caption = 'Echo Name'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object LabelEchoDate: TLabel
    Left = 80
    Top = 32
    Width = 225
    Height = 17
    AutoSize = False
    Caption = 'LabelEchoDate'
  end
  object Label3: TLabel
    Left = 8
    Top = 32
    Width = 51
    Height = 13
    AutoSize = False
    Caption = 'Echo Date'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object LabelEchoName: TLabel
    Left = 80
    Top = 8
    Width = 233
    Height = 21
    AutoSize = False
    Caption = 'LabelEchoName'
  end
  object LabelExplain: TLabel
    Left = 8
    Top = 56
    Width = 329
    Height = 121
    AutoSize = False
    Caption = 'LabelExplain'
  end
  object Button0: TButton
    Left = 8
    Top = 184
    Width = 329
    Height = 25
    Caption = 'Button0'
    TabOrder = 0
    OnClick = Button0Click
  end
  object Button1: TButton
    Left = 8
    Top = 216
    Width = 329
    Height = 25
    Caption = 'Button1'
    TabOrder = 1
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 8
    Top = 248
    Width = 329
    Height = 25
    Caption = 'Button2'
    TabOrder = 2
    OnClick = Button2Click
  end
  object Button5: TButton
    Left = 8
    Top = 344
    Width = 329
    Height = 25
    Caption = 'Button5'
    TabOrder = 3
    OnClick = Button5Click
  end
  object Button6: TButton
    Left = 8
    Top = 376
    Width = 329
    Height = 25
    Caption = 'Button6'
    TabOrder = 4
    OnClick = Button6Click
  end
  object Button7: TButton
    Left = 8
    Top = 408
    Width = 329
    Height = 25
    Caption = 'Button7'
    TabOrder = 5
    OnClick = Button7Click
  end
  object Button8: TButton
    Left = 8
    Top = 440
    Width = 329
    Height = 25
    Caption = 'Button8'
    TabOrder = 6
    OnClick = Button8Click
  end
  object Button9: TButton
    Left = 8
    Top = 472
    Width = 329
    Height = 25
    Caption = 'Button9'
    TabOrder = 7
    OnClick = Button9Click
  end
  object Button4: TButton
    Left = 8
    Top = 312
    Width = 329
    Height = 25
    Caption = 'Button4'
    TabOrder = 8
    OnClick = Button4Click
  end
  object ButtonCancel: TButton
    Left = 264
    Top = 528
    Width = 75
    Height = 25
    Caption = 'Cancel'
    TabOrder = 9
    OnClick = ButtonCancelClick
  end
  object Button3: TButton
    Left = 8
    Top = 280
    Width = 329
    Height = 25
    Caption = 'Button3'
    TabOrder = 10
    OnClick = Button3Click
  end
  object ButtonNoMatch: TButton
    Left = 8
    Top = 528
    Width = 171
    Height = 25
    Caption = 'No Match'
    TabOrder = 11
    OnClick = ButtonNoMatchClick
  end
end
