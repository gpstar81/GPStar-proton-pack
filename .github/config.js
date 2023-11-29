module.exports = {
	marked_options: {
		headerIds: false,
		smartypants: true,
	},
	pdf_options: {
		format: 'A4',
		margin: '30mm 20mm',
		printBackground: true,
    displayHeaderFooter: true,
    headerTemplate: "<div style='margin:30px 60px 0; width: 100%; font-size: 11px; border-bottom: solid lightgray 1px;'> <div class='date' style='float: right'></div> </div>",
    footerTemplate: "<div style='margin:0 60px 30px; width: 100%; font-size: 9px; border-top: solid lightgray 1px;'> <div style='margin: 0 auto; width: fit-content;'> <span class='pageNumber'></span> / <span class='totalPages'></span> </div></div>"
	},
	stylesheet_encoding: 'utf-8'
};
