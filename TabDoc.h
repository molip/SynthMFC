#pragma once

class TabDoc : public CDocument
{
protected: 
	TabDoc();
	DECLARE_DYNCREATE(TabDoc)

public:
	static TabDoc* Instance();

public:
	virtual BOOL OnNewDocument() override;
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName) override;
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName) override;

public:
	virtual ~TabDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	DECLARE_MESSAGE_MAP()
};
