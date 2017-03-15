#pragma once

#include <memory>

namespace Synth
{
	namespace UI
	{
		class Controller;
	}
}

class CSynthEditorDoc : public CDocument
{
protected: // create from serialization only
	CSynthEditorDoc();
	DECLARE_DYNCREATE(CSynthEditorDoc)

public:
	static CSynthEditorDoc* Instance();

	Synth::UI::Controller& GetController() { return *_controller; }

private:
	void CSynthEditorDoc::CreateController();

	std::unique_ptr<Synth::UI::Controller> _controller;

// Overrides
public:
	virtual BOOL OnNewDocument() override;
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName) override;
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName) override;

#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CSynthEditorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
